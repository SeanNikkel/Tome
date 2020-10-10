// Fill out your copyright notice in the Description page of Project Settings.

#include "Book.h"

// Sets default values
ABook::ABook()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	RootComponent = SceneRoot;

	FrontMesh = CreateDefaultSubobject<UStaticMeshComponent>("FrontCover");
	FrontText = CreateDefaultSubobject<UTextRenderComponent>("FrontText");
	FrontPageNum = CreateDefaultSubobject<UTextRenderComponent>("FrontPageNum");
	FrontSpineText = CreateDefaultSubobject<UTextRenderComponent>("FrontSpineText");

	BackMesh = CreateDefaultSubobject<UStaticMeshComponent>("BackCover");
	BackText = CreateDefaultSubobject<UTextRenderComponent>("BackText");
	BackPageNum = CreateDefaultSubobject<UTextRenderComponent>("BackPageNum");
	BackSpineText = CreateDefaultSubobject<UTextRenderComponent>("BackSpineText");

	CoverText = CreateDefaultSubobject<UTextRenderComponent>("CoverText");

	FrontMesh->SetupAttachment(SceneRoot);
	FrontText->SetupAttachment(FrontMesh);
	FrontPageNum->SetupAttachment(FrontMesh);
	FrontSpineText->SetupAttachment(FrontMesh);

	BackMesh->SetupAttachment(SceneRoot);
	BackText->SetupAttachment(BackMesh);
	BackPageNum->SetupAttachment(BackMesh);
	BackSpineText->SetupAttachment(BackMesh);

	CoverText->SetupAttachment(FrontMesh);
}

// Called when the game starts or when spawned
void ABook::BeginPlay()
{
	Super::BeginPlay();
}

char ABook::GetRandomCharacter(bool punctuation)
{
	char possibleCharacters[] = "abcdefghijklmnopqrstuvwxyz .,";

	int32 range = sizeof(possibleCharacters) / sizeof(possibleCharacters[0]) - 2; // -1 for inclusive, -1 for null

    // Last two are punctuation
	if (!punctuation)
		range -= 2;

    // Return random from remaining
	return possibleCharacters[stream.RandRange(0, range)];
}

const FString &ABook::GetPage(int32 page)
{
	static FString empty = "";

    // Out of bounds
	if (page < 1 || page > pageCount)
		return empty;

    // Number of characters to generate
	float characters = pageLineLength * pageLineCount;

	// Last page could have less characters
	if (page == pageCount)
		characters = stream.RandRange(1, characters);

    // Generate the page if doesn't exist
	if (!pageContents.Contains(page))
		pageContents.Add(page, GenerateText(characters, pageLineLength));

	return pageContents[page];
}

FString ABook::GetPageNumber(int32 page)
{
	if (page < 1 || page > pageCount)
		return "";
	
	return FString::FromInt(page);
}

FString ABook::GenerateText(int32 length, int32 lineSize, bool punctuation)
{
	FString result;

	for (int32 i = 0; i < length; i++)
	{
		result += GetRandomCharacter(punctuation);

        // Newlines after every lineSize
		if (lineSize != 0 && (i + 1) % lineSize == 0)
			result += '\n';
	}

	return result;
}

void ABook::WrapString(FString &string, int32 lineLength)
{
	TArray<FString> split = DivideString(string, " ");
	int32 currentLength = 0;
	string = "";

    // For each word
	for (int i = 0; i < split.Num(); i++)
	{
        // If the word fits, add it
		if (currentLength + split[i].Len() <= lineLength)
		{
			string += split[i] + ' ';
			currentLength += split[i].Len() + 1;
		}
		else
		{
            // If there is something on this line already, go to next line
			if (currentLength != 0)
			{
				string.TrimEndInline();
				string += '\n';
				currentLength = 0;
			}
			else // split this word with a dash, add left side
			{
				string += split[i].Left(lineLength - 1) + "-\n";
				split[i] = split[i].RightChop(lineLength - 1);
			}
			i--; // redo this word
		}
	}
	string.TrimEndInline();
}

void ABook::TitleCase(FString &string)
{
	if (string.Len() == 0)
		return;

    // Capitalize first letter
	string[0] = ToUpper(string[0]);

	for (int i = 1; i < string.Len(); i++)
	{
        // Capitalize letters after spaces
		if (string[i - 1] == ' ')
			string[i] = ToUpper(string[i]);
	}
}

char ABook::ToUpper(char c)
{
	if (c >= 'a' && c <= 'z')
		c += ('A' - 'a');
	
	return c;
}

TArray<FString> ABook::DivideString(FString string, const FString &delimiter)
{
	TArray<FString> result;
	FString left;
	FString right = string;

    // Split string until out of characters
	while (string.Split(delimiter, &left, &right))
	{
		result.Add(left);
		string = right;
	}
	result.Add(right);

	return result;
}

void ABook::RemoveSequentialString(FString &string, TCHAR character)
{
	int32 found;
	int32 index = 0;

    // Find character after index
	while (string.RightChop(index).FindChar(character, found))
	{
		index += found + 1;
		int32 toRemove = 0;

        // Get all sequential characters
		while (index < string.Len() && string[index] == character)
		{
			toRemove++;
			index++;
		}

        // Remove them
		string.RemoveAt(index - toRemove, toRemove);
		index -= toRemove;
	}
}

// Called every frame
void ABook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABook::SetSeed(int32 seed)
{
	stream.Initialize(seed);
}

void ABook::GenerateOuterText()
{
    // Generate title
	FString outerContent = GenerateText(stream.RandRange(1, coverMaxLength), 0, false);
	outerContent.TrimStartAndEndInline();
	RemoveSequentialString(outerContent, ' ');
	TitleCase(outerContent);

    // Spine title
	FrontSpineText->SetText(FText::FromString(outerContent));
	BackSpineText->SetText(FText::FromString(outerContent));

    // Front title
	WrapString(outerContent, coverLineLength);
	CoverText->SetText(FText::FromString(outerContent));
}

void ABook::DisplayPage(int32 page, USoundBase *sound)
{
	if (page < 1 || page > pageCount)
		return;

    // Play flip sound
	if (sound != nullptr)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), sound, GetActorLocation());

    // Set page text
	FrontText->SetText(FText::FromString(GetPage(page)));
	BackText->SetText(FText::FromString(GetPage(page + 1)));

    // Set page numbers
	FrontPageNum->SetText(FText::FromString(GetPageNumber(page)));
	BackPageNum->SetText(FText::FromString(GetPageNumber(page + 1)));

	currentPage = page;
}

