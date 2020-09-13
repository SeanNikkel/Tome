// Fill out your copyright notice in the Description page of Project Settings.

#define EPSILON 0.001f

#include "BookRow.h"

// Sets default values
ABookRow::ABookRow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

bool ABookRow::AddBook(ABook *book, FVector worldPos, FVector &outPos, bool enforceMaxDist)
{
	return AddBookLocal(book, GetTransform().InverseTransformPosition(worldPos).Y, outPos, enforceMaxDist);
}

bool ABookRow::AddBookLocal(ABook *book, float localPosY, FVector &outPos, bool enforceMaxDist)
{
	ValidateBookArray();

	float bound = width / 2.0f - book->halfWidth;
	localPosY = FMath::Clamp(localPosY, -bound, bound);

	// Find position to be inserted before
	int32 index = GetIndex(localPosY);

	float position = localPosY;
	float leftPos = GetBoundaryRight(index) - book->halfWidth;
	int32 leftIndex = index;
	float rightPos = GetBoundaryLeft(index) + book->halfWidth;
	int32 rightIndex = index;

	// If book does not fit
	if (leftPos < position || rightPos > position)
	{
		while (true)
		{
			if (leftIndex < 0 && rightIndex > books.Num())
				return false;

			// Left pos is closer, or decide based on left/right index valid
			if (leftIndex >= 0 && (rightIndex > books.Num() || FMath::Abs(localPosY - leftPos) < FMath::Abs(localPosY - rightPos)))
			{
				if (enforceMaxDist && FMath::Abs(localPosY - leftPos) > maxSnapDistance)
					return false;

				// If fits
				if (GetBoundaryLeft(leftIndex) - EPSILON <= leftPos - book->halfWidth)
				{
					position = leftPos;
					index = leftIndex;
					break;
				}
				leftIndex--;
				leftPos = GetBoundaryRight(leftIndex) - book->halfWidth;
			}
			else
			{
				if (enforceMaxDist && FMath::Abs(localPosY - rightPos) > maxSnapDistance)
					return false;

				// If fits
				if (GetBoundaryRight(rightIndex) + EPSILON >= rightPos + book->halfWidth)
				{
					position = rightPos;
					index = rightIndex;
					break;
				}
				rightIndex++;
				rightPos = GetBoundaryLeft(rightIndex) + book->halfWidth;
			}
		}
	}

	outPos = AddBookRaw(book, index, position);

	return true;
}

void ABookRow::RemoveBook(ABook *book)
{
	books.Remove(book);
}

void ABookRow::GenerateBooksSimple(int32 count, ABookPool *bookPool)
{
	books.Reserve(count);
	for (int i = 0; i < count; i++)
	{
		ABook *book;

		if (bookPool != nullptr)
			book = bookPool->GetBook();
		else
			book = Cast<ABook>(GetWorld()->SpawnActor(bookType.Get()));

		float position = FMath::RandRange(-width / 2.0f + book->halfWidth, width / 2.0f - book->halfWidth);
		//FVector world = AddBookRaw(book, GetIndex(position), position);
		FVector local;
		if (!AddBookLocal(book, position, local, false))
		{
			book->Destroy();
			break;
		}
		else
		{
			book->SetActorRelativeLocation(local);
			book->SetActorRelativeRotation(FQuat::MakeFromEuler(FVector(0.0f, 270.0f, 0.0f)));
		}
	}
}

void ABookRow::GenerateBooks(ABookPool *bookPool)
{
	int32 groupSizeMin = 1;
	int32 groupSizeMax = 16;

	float spaceBetweenMin = 1.0f;
	float spaceBetweenMax = 100.0f;

	float leaningBookSize = 40.0f;
	float leaningBookChance = 0.2f;
	float leaningBookAngle = 20.0f;

	float flatBookChance = 0.1f;
	float flatBookSize = 75.0f;

	float endGroupChance = 0.8f;



	float leftBorder = -width / 2.0f;
	float rightBorder = width / 2.0f;

	if (FMath::FRand() < endGroupChance)
		leftBorder = AddGroup(leftBorder, FMath::RandRange(groupSizeMin, groupSizeMax), rightBorder, true, bookPool);

	if (FMath::FRand() < endGroupChance)
		rightBorder = AddGroup(rightBorder, FMath::RandRange(groupSizeMin, groupSizeMax), leftBorder, false, bookPool);

	if (rightBorder == FP_NAN)
		return;

	while (true)
	{
		leftBorder += FMath::RandRange(spaceBetweenMin, spaceBetweenMax);

		if (leftBorder >= rightBorder)
			break;

		if (FMath::FRand() < leaningBookChance && leftBorder + leaningBookSize < rightBorder)
		{
			ABook *book = bookPool->GetBook();
			book->AttachToActor(this, { EAttachmentRule::KeepRelative, false });
			book->EnablePhysics(true);
			book->PhysicsTeleport(FVector(0.0f, leftBorder + leaningBookSize / 2.0f, 0.0f), FRotator(270.0f + leaningBookAngle, 270.0f, 90.0f));
			leftBorder += leaningBookSize;
		}

		if (FMath::FRand() < flatBookChance && leftBorder + flatBookSize < rightBorder)
		{
			ABook *book = bookPool->GetBook();
			book->AttachToActor(this, { EAttachmentRule::KeepRelative, false });
			book->EnablePhysics(true);
			book->PhysicsTeleport(FVector(0.0f, leftBorder + flatBookSize * 0.75f, 0.0f), FRotator(180.0f, 270.0f, 90.0f));
			leftBorder += leaningBookSize;
		}
		else
			leftBorder = AddGroup(leftBorder, FMath::RandRange(groupSizeMin, groupSizeMax), rightBorder, true, bookPool);

		if (leftBorder == FP_NAN)
			break;

		if (FMath::FRand() < leaningBookChance && leftBorder + leaningBookSize < rightBorder)
		{
			ABook *book = bookPool->GetBook();
			book->AttachToActor(this, { EAttachmentRule::KeepRelative, false });
			book->EnablePhysics(true);
			book->PhysicsTeleport(FVector(0.0f, leftBorder + leaningBookSize / 2.0f, 0.0f), FRotator(270.0f - leaningBookAngle, 270.0f, 90.0f));
			leftBorder += leaningBookSize;
		}
	}
}

// Called when the game starts or when spawned
void ABookRow::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABookRow::ValidateBookArray()
{
	TArray<AActor *> children;
	GetAttachedActors(children);

	for (int i = 0; i < books.Num(); i++)
	{
		if (children.Find(books[i]) == INDEX_NONE)
		{
			books.RemoveAt(i);
			i--;
		}
	}
}

float ABookRow::GetBoundaryLeft(int32 indexBefore)
{
	if (indexBefore <= 0)
		return -width / 2.0f;
	else if (indexBefore > books.Num())
		return width / 2.0f;
	else
		return books[indexBefore - 1]->GetRootComponent()->GetRelativeLocation().Y + books[indexBefore - 1]->halfWidth;
}

float ABookRow::GetBoundaryRight(int32 indexBefore)
{
	if (indexBefore < 0)
		return -width / 2.0f;
	else if (indexBefore >= books.Num())
		return width / 2.0f;
	else
		return books[indexBefore]->GetRootComponent()->GetRelativeLocation().Y - books[indexBefore]->halfWidth;
}

FVector ABookRow::AddBookRaw(ABook *book, int32 index, float position)
{
	books.Insert(book, index);
	book->AttachToActor(this, { EAttachmentRule::KeepWorld, false });
	return FVector(0.0f, position, 0.0f);
}

int32 ABookRow::GetIndex(float position)
{
	int32 index;
	for (index = 0; index < books.Num(); index++)
	{
		if (books[index]->GetRootComponent()->GetRelativeLocation().Y > position)
			break;
	}
	return index;
}

float ABookRow::AddGroup(float position, int32 count, float border, bool goingRight, ABookPool *bookPool)
{
	int32 dir = goingRight ? 1 : -1;
	int32 index = -1;

	for (int32 i = 0; i < count; i++)
	{
		ABook *book = bookPool->GetBook();

		position += book->halfWidth * dir;

		if (goingRight && position + book->halfWidth > border ||
			!goingRight && position - book->halfWidth < border)
		{
			book->Destroy();
			return FP_NAN;
		}

		if (index == -1)
			index = GetIndex(position);
		else if (goingRight)
			index++;

		FVector pos = AddBookRaw(book, index, position);
		book->SetActorRelativeLocation(pos);
		book->SetActorRelativeRotation(FQuat::MakeFromEuler(FVector(0.0f, 270.0f, 0.0f)));

		position += book->halfWidth * dir;
	}

	return position;
}

// Called every frame
void ABookRow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABookRow::Destroyed()
{
	TArray<AActor *> children;
	GetAttachedActors(children);

	for (AActor *child : children)
	{
		child->Destroy();
	}
}

