// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Book.h"
#include "BookPool.generated.h"

UCLASS()
class TOME_API ABookPool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABookPool();

    // Get a book from book pool
	UFUNCTION(BlueprintImplementableEvent)
	ABook *GetBook();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
