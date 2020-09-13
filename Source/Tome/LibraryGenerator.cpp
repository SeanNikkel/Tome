// Fill out your copyright notice in the Description page of Project Settings.


#include "LibraryGenerator.h"

const FIntVector ALibraryGenerator::directions[] = {
	FIntVector(-1, 0, 0),
	FIntVector(1, 0, 0),
	FIntVector(0, -1, 0),
	FIntVector(0, 1, 0),
	FIntVector(0, 0, -1),
	FIntVector(0, 0, 1)
};

const float ALibraryGenerator::rotations[] = {
	0.0f,
	90.0f,
	180.0f,
	270.0f
};

// Sets default values
ALibraryGenerator::ALibraryGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALibraryGenerator::BeginPlay()
{
	Super::BeginPlay();
}

FIntVector ALibraryGenerator::WorldToGrid(FVector world)
{
	world /= gridSize;
	return FIntVector(FMath::RoundToInt(world.X), FMath::RoundToInt(world.Y), FMath::RoundToInt(world.Z));
}

FVector ALibraryGenerator::GridToWorld(FIntVector grid)
{
	return FVector(grid) * gridSize;
}

ETileDirection ALibraryGenerator::ReverseDirection(ETileDirection direction)
{
	return ETileDirection(uint8(direction) + ((uint8(direction) % 2) ? -1 : 1));
}

ETileDirection ALibraryGenerator::RotateDirection(ETileDirection direction, ETileRotation rotation)
{
	if (direction == ETileDirection::TD_ABOVE || direction == ETileDirection::TD_BELOW || rotation == ETileRotation::ROT_0)
		return direction;

	if (rotation == ETileRotation::ROT_180)
		return ReverseDirection(direction);

	if (rotation == ETileRotation::ROT_90)
	{
		switch (direction)
		{
		case ETileDirection::TD_LEFT:
			return ETileDirection::TD_BACK;
		case ETileDirection::TD_RIGHT:
			return ETileDirection::TD_FRONT;
		case ETileDirection::TD_BACK:
			return ETileDirection::TD_RIGHT;
		case ETileDirection::TD_FRONT:
			return ETileDirection::TD_LEFT;
		}
	}

	if (rotation == ETileRotation::ROT_270)
	{
		switch (direction)
		{
		case ETileDirection::TD_LEFT:
			return ETileDirection::TD_FRONT;
		case ETileDirection::TD_RIGHT:
			return ETileDirection::TD_BACK;
		case ETileDirection::TD_BACK:
			return ETileDirection::TD_LEFT;
		case ETileDirection::TD_FRONT:
			return ETileDirection::TD_RIGHT;
		}
	}

	return ETileDirection::TD_MAX;
}

ETileDirection ALibraryGenerator::ScaleDirection(ETileDirection direction, FVector scale)
{
	if (scale.X < 0.0f)
	{
		if (direction == ETileDirection::TD_LEFT)
			return ETileDirection::TD_RIGHT;
		if (direction == ETileDirection::TD_RIGHT)
			return ETileDirection::TD_LEFT;
	}
	if (scale.Y < 0.0f)
	{
		if (direction == ETileDirection::TD_BACK)
			return ETileDirection::TD_FRONT;
		if (direction == ETileDirection::TD_FRONT)
			return ETileDirection::TD_BACK;
	}
	if (scale.Z < 0.0f)
	{
		if (direction == ETileDirection::TD_BELOW)
			return ETileDirection::TD_ABOVE;
		if (direction == ETileDirection::TD_ABOVE)
			return ETileDirection::TD_BELOW;
	}

	return direction;
}

ETileRotation ALibraryGenerator::ReverseRotation(ETileRotation rotation)
{
	if (rotation == ETileRotation::ROT_90)
		return ETileRotation::ROT_270;

	if (rotation == ETileRotation::ROT_270)
		return ETileRotation::ROT_90;

	return rotation;
}

ETileConnection ALibraryGenerator::GetConnection(const FTileInfo *info, ETileDirection direction, ETileRotation rotation, FVector scale)
{
	return info->connections[uint8(ScaleDirection(RotateDirection(direction, ReverseRotation(rotation)), scale))];
}

ETileRotation ALibraryGenerator::ActorTileRotation(AActor *actor)
{
	return ETileRotation(PositiveMod(FMath::RoundToInt(actor->GetActorRotation().Yaw / 90.0f), uint32(ETileRotation::ROT_MAX)));
}

int32 ALibraryGenerator::PositiveMod(int32 value, int32 mod)
{
	if (value >= 0)
		return value % mod;
	else
		return (1 - (value + 1) / mod) * mod + value;
}

void ALibraryGenerator::GenerateTile(FIntVector coord)
{
	// Array of possible tiles+rotations to generate
	TArray<RotatedTile> possibilities;

	// Try all tiles
	for (const TPair<FName, uint8*> &row : tileData->GetRowMap())
	{
		const FTileInfo *info = reinterpret_cast<FTileInfo *>(row.Value);

		// Check for spawn tiles
		if (coord == FIntVector(0, 0, 0) && !info->canSpawnOn)
			continue;

		// Check for blacklist
		if (info->blacklisted.Num() != 0)
		{
			bool blacklisted = false;
			for (uint8 d = 0; d < uint8(ETileDirection::TD_MAX); d++)
			{
				TileInstance *adjacent = tiles_.Find(coord + directions[d]);

				if (adjacent == nullptr || adjacent->actor == nullptr)
					continue;

				if (info->blacklisted.Contains(adjacent->info->object))
				{
					blacklisted = true;
					break;
				}
			}
			if (blacklisted)
				continue;
		}

		// Try all tile rotations
		for (uint8 r = 0; r < uint8(ETileRotation::ROT_MAX); r++)
		{
			FVector scales[] = { FVector(1, 1, 1), FVector(-1, 1, 1) };
			uint8 scaleNum = sizeof(scales) / sizeof(scales[0]);

			// Check if need to mirror
			if (!info->mirror)
				scaleNum = 1;

			for (uint8 s = 0; s < scaleNum; s++)
			{
				bool valid = true;

				// Check that in every direction, connections match surrounding tiles
				for (uint8 d = 0; d < uint8(ETileDirection::TD_MAX); d++)
				{
					TileInstance *adjacent = tiles_.Find(coord + directions[d]);

					// If this tile hasn't loaded yet, ignore
					if (adjacent == nullptr)
						continue;

					// Get this tile's connection in adjacent's direction
					ETileConnection connection = GetConnection(info, ETileDirection(d), ETileRotation(r), scales[s]);

					// Get connection in this direction from adjacent tile, or empty if adjacent is empty
					ETileConnection adjacentConnection = ETileConnection::TC_EMPTY;
					ETileRotation adjacentRotation = ETileRotation(r);
					FVector adjacentScale = scales[s];
					if (adjacent->actor != nullptr)
					{
						adjacentRotation = ActorTileRotation(adjacent->actor);
						adjacentScale = adjacent->actor->GetActorScale3D();
						adjacentConnection = GetConnection(adjacent->info, ReverseDirection(ETileDirection(d)), adjacentRotation, adjacentScale);
					}

					// If the connections don't match or stacked vertically and rotations don't match, stop and don't add this as a possibility
					bool vertRotCheck = (d == uint8(ETileDirection::TD_ABOVE) || d == uint8(ETileDirection::TD_BELOW)) && connection != ETileConnection::TC_EMPTY && (adjacentRotation != ETileRotation(r) || adjacentScale != scales[s]);
					if (connection != adjacentConnection || vertRotCheck)
					{
						valid = false;
						break;
					}
				}

				// Only add tile if all directions passed
				if (valid)
					possibilities.Add({ info, ETileRotation(r), scales[s] });
			}

		}
	}

	// No possible tiles for this space
	if (possibilities.Num() == 0)
	{
		// Add empty
		tiles_.Add(coord, {});
	}
	else
	{
		// Add random possibility
		const RotatedTile &result = possibilities[FMath::RandRange(0, possibilities.Num() - 1)];
		AddTile(coord, result.rot, result.scale, result.info);
	}
}

void ALibraryGenerator::AddTile(FIntVector coord, ETileRotation rotation, FVector scale, const FTileInfo *info)
{
	// Spawn actor
	FVector pos = GridToWorld(coord);
	FRotator rot(0.0f, rotations[uint8(rotation)], 0.0f);
	AActor *actor = GetWorld()->SpawnActor(info->object.Get(), &pos, &rot);
	actor->AttachToActor(geometryParent, { EAttachmentRule::KeepRelative, false });
	actor->SetActorScale3D(scale);

	// Reset render state after scaling (thanks unreal)
	actor->SetActorHiddenInGame(true);
	actor->SetActorHiddenInGame(false);

	// Add to data structure
	tiles_.Add(coord, { actor, info });
}

void ALibraryGenerator::UnloadTile(FIntVector coord)
{
	// Remove from tile map
	TileInstance tile;
	tiles_.RemoveAndCopyValue(coord, tile);

	// Destroy actor
	if (tile.actor)
		GetWorld()->DestroyActor(tile.actor);
}

// Called every frame
void ALibraryGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Get player position
	FVector playerPosF = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
	FIntVector playerPos = WorldToGrid(playerPosF);

	// Get positive corner vector of grid cube to check tiles in
	FVector cubeCornerF = FVector(renderDistance) / gridSize;
	FIntVector cubeCorner = FIntVector(FMath::CeilToInt(cubeCornerF.X), FMath::CeilToInt(cubeCornerF.Y), FMath::CeilToInt(cubeCornerF.Z));

	// List of coordinates to generate tiles in
	TArray<FIntVector> coordsToLoad;

	// Get coords to load
	for (int32 z = playerPos.Z - cubeCorner.Z; z <= playerPos.Z + cubeCorner.Z; z++)
	{
		for (int32 y = playerPos.Y - cubeCorner.Y; y <= playerPos.Y + cubeCorner.Y; y++)
		{
			for (int32 x = playerPos.X - cubeCorner.X; x <= playerPos.X + cubeCorner.X; x++)
			{
				FIntVector current(x, y, z);
				TileInstance *tile = tiles_.Find(current);

				// If in range
				if (FVector::Distance(GridToWorld(current), playerPosF) <= renderDistance)
				{
					// Add to load list
					if (tile == nullptr)
						coordsToLoad.Add(current);
				}
				else
				{
					// Unload
					if (tile != nullptr)
						UnloadTile(current);
				}

				if (debugGridDraw)
					DrawDebugBox(GetWorld(), GridToWorld(current), gridSize / 2.0f, FColor(0), false, 1/50.0f);
			}
		}
	}

	// Sort by distance to player
	coordsToLoad.Sort([&, playerPosF](const FIntVector &a, const FIntVector &b) { return FVector::Distance(playerPosF, GridToWorld(a)) < FVector::Distance(playerPosF, GridToWorld(b)); });

	// Load valid tiles in sorted order
	for (const FIntVector &coord : coordsToLoad)
	{
		GenerateTile(coord);
	}
}

