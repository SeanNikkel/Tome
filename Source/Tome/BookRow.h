// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Book.h"
#include "BookPool.h"
#include "BookRow.generated.h"

UCLASS()
class TOME_API ABookRow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABookRow();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	// Returns whether book could be placed.
	// If true, outPos contains coordinates in BookRow's space for the book to be placed
	UFUNCTION(BlueprintCallable)
	bool AddBook(ABook *book, FVector worldPos, FVector &outPos, bool enforceMaxDist = true);

	// Addbook with local input
	UFUNCTION(BlueprintCallable)
	bool AddBookLocal(ABook *book, float localPosY, FVector &outPos, bool enforceMaxDist = true);
	
	// Removes a book from the row
	UFUNCTION(BlueprintCallable)
	void RemoveBook(ABook *book);

    // Generate books randomly
	UFUNCTION(BlueprintCallable)
	void GenerateBooksSimple(int32 count, ABookPool *bookPool = nullptr);

    // Generate books in groups
	UFUNCTION(BlueprintCallable)
	void GenerateBooks(ABookPool *bookPool = nullptr);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Remove any books that aren't children
	void ValidateBookArray();

    // Get book edge positions
	float GetBoundaryLeft(int32 indexBefore);
	float GetBoundaryRight(int32 indexBefore);

	// Return world coordinates of book added
	FVector AddBookRaw(ABook *book, int32 index, float position);

    // Get book at position
	int32 GetIndex(float position);

    // Add a group of books to the shelf
	float AddGroup(float position, int32 count, float border, bool goingRight = true, ABookPool *bookPool = nullptr);

public:	
	UPROPERTY(BlueprintReadOnly)
	UBoxComponent *Collider;

	UPROPERTY(BlueprintReadWrite)
	float width;

	UPROPERTY(BlueprintReadWrite)
	float maxSnapDistance = 50.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<ABook> bookType;

private:
	TArray<ABook *> books;
};
