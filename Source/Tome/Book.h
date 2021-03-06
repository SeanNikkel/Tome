// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h" 
#include "Book.generated.h"

UCLASS()
class TOME_API ABook : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABook();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set the seed for this book
	UFUNCTION(BlueprintCallable)
	void SetSeed(int32 seed);

	// Generate and display text for outside of book (spine and cover)
	UFUNCTION(BlueprintCallable)
	void GenerateOuterText();

	// Displays page given on left and page after on right. Generates first if needed
	UFUNCTION(BlueprintCallable)
	void DisplayPage(int32 page, USoundBase *sound = nullptr);

    // Event for blueprint to enable physics
	UFUNCTION(BlueprintImplementableEvent)
	void EnablePhysics(bool enable);

    // Event for blueprint to teleport the book
	UFUNCTION(BlueprintImplementableEvent)
	void PhysicsTeleport(FVector localPos, FRotator localRot);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
    // Returns a random character
	char GetRandomCharacter(bool punctuation = true);

    // Get content on a page (generate if doesn't exist)
	const FString &GetPage(int32 page);

    // Get the page number as a string
	FString GetPageNumber(int32 page);

    // Generate text for a page
	FString GenerateText(int32 length, int32 lineSize = 0, bool punctuation = true);

    // Insert newlines so string fits width
	void WrapString(FString &string, int32 lineLength);

    // Convert string to first letters capitalized
	void TitleCase(FString &string);

    // Convert char to uppercase
	char ToUpper(char c);

    // Divide string into array of strings given delimiter
	TArray<FString> DivideString(FString string, const FString &delimiter);

    // Remove repeated characters in string
	void RemoveSequentialString(FString &string, TCHAR character);

public:
	
	UPROPERTY(BlueprintReadOnly)
	int32 currentPage = 1;

	// Generation

	UPROPERTY(BlueprintReadOnly)
	FRandomStream stream;

	UPROPERTY(BlueprintReadOnly)
	TMap<int32, FString> pageContents;
	
	UPROPERTY(BlueprintReadWrite)
	int32 pageLineCount = 15;

	UPROPERTY(BlueprintReadWrite)
	int32 coverMaxLength = 16;

	UPROPERTY(BlueprintReadWrite)
	int32 pageLineLength = 24;

	UPROPERTY(BlueprintReadWrite)
	int32 coverLineLength = 8;

	UPROPERTY(BlueprintReadWrite)
	int32 pageCount = 410;

	UPROPERTY(BlueprintReadWrite)
	float halfWidth = 0.0f;

	// Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent *SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent *FrontMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent *FrontText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent *FrontPageNum;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent *FrontSpineText;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent *BackMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent *BackText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent *BackPageNum;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent *BackSpineText;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent *CoverText;
};
