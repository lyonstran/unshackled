# Unshackled reference links

## Unreal Engine (official)
- Documentation home: https://dev.epicgames.com/documentation/en-us/unreal-engine/
- C++ API reference home: https://dev.epicgames.com/documentation/en-us/unreal-engine/API
- Programming and scripting: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-cpp-programming-tutorials
- Gameplay framework: https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-framework-in-unreal-engine
- Actors (AActor, spawning, Tick): https://dev.epicgames.com/documentation/en-us/unreal-engine/actors-in-unreal-engine
- Characters and CharacterMovementComponent: https://dev.epicgames.com/documentation/en-us/unreal-engine/characters-in-unreal-engine
- Timers (SetTimer, named UFUNCTIONs): https://dev.epicgames.com/documentation/en-us/unreal-engine/using-timers-in-unreal-engine
- Collision, overlaps and traces: https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine
- Enhanced Input: https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine
- Data Tables (FTableRowBase): https://dev.epicgames.com/documentation/en-us/unreal-engine/data-driven-gameplay-elements-in-unreal-engine
- UPROPERTY specifiers: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-uproperties
- UFUNCTION specifiers: https://dev.epicgames.com/documentation/en-us/unreal-engine/ufunctions-in-unreal-engine
- Gameplay classes and the class system: https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-classes-in-unreal-engine
- Coding standard (naming, prefixes A/U/F/E): https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine
- HUD and canvas drawing (AHUD: DrawRect, Project): https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/AHUD
- Debug drawing (UKismetSystemLibrary, see DrawDebugHelpers.h): https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UKismetSystemLibrary
- Live Coding: https://dev.epicgames.com/documentation/en-us/unreal-engine/using-live-coding-to-recompile-unreal-engine-applications-at-runtime
- Paper 2D overview (sprites, flipbooks, UPaperFlipbookComponent): https://dev.epicgames.com/documentation/en-us/unreal-engine/paper-2d-overview-in-unreal-engine
- Epic Learning Library (search "git lfs unreal"): https://dev.epicgames.com/community/unreal-engine/learning

## Functions used in the project (search the API reference for each)
- UWorld::SpawnActor, OverlapMultiByObjectType, GetTimerManager
- AActor::SetActorLocation, Destroy, SetLifeSpan, GetActorLocation
- TActorIterator (EngineUtils.h)
- FVector: Dist2D, Normalize, IsNearlyZero, Rotation
- FMath::LinePlaneIntersection
- APlayerController::DeprojectMousePositionToWorld, GetInputMouseDelta
- UCharacterMovementComponent: SetMovementMode, MaxFlySpeed, LaunchCharacter
- UEnhancedInputComponent::BindAction
- UPaperFlipbookComponent (Paper2D)
- GEngine->AddOnScreenDebugMessage
- FString::Printf

## PaperZD
- PaperZD documentation: https://github.com/heavybullets/PaperZD/wiki

## Community and Q&A
- Epic developer community forums: https://forums.unrealengine.com/
- Tom Looman, C++ and gameplay articles: https://www.tomlooman.com/
- Ben UI, Unreal C++ notes: https://benui.ca/unreal/

## C++ basics 
- cppreference: https://en.cppreference.com/
- Learn C++: https://www.learncpp.com/

## Git
- Pro Git book: https://git-scm.com/book/en/v2
- Git LFS: https://git-lfs.com/

## Custom actor assignment (exit door)
- Components overview: https://dev.epicgames.com/documentation/en-us/unreal-engine/components-in-unreal-engine
- Input overview (UInputComponent, BindKey): https://dev.epicgames.com/documentation/en-us/unreal-engine/input-in-unreal-engine
- UInputComponent API: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UInputComponent
- Setting up input on an Actor (EnableInput): https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-input-on-an-actor-in-unreal-engine
- UStaticMeshComponent API: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UStaticMeshComponent
- UPointLightComponent API: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UPointLightComponent
- USceneComponent API (SetupAttachment, AttachToComponent): https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/USceneComponent
- UMaterialInstanceDynamic API (SetVectorParameterValue): https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UMaterialInstanceDynamic
- Instanced materials: https://dev.epicgames.com/documentation/en-us/unreal-engine/instanced-materials-in-unreal-engine
- ConstructorHelpers::FObjectFinder: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/ConstructorHelpers
- Creating objects (NewObject) at runtime: https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-objects-in-unreal-engine
- UGameplayStatics (OpenLevel, GetPlayerPawn, GetPlayerController): https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UGameplayStatics
- UGameInstance (data that survives OpenLevel): https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UGameInstance
- Loading and unloading levels with C++ (OpenLevel): https://dev.epicgames.com/documentation/en-us/unreal-engine/loading-and-unloading-levels-using-cplusplus-in-unreal-engine
- UE_LOG and logging: https://dev.epicgames.com/documentation/en-us/unreal-engine/logging-in-unreal-engine
- Unreal reflection and UObject garbage collection (why pointers need UPROPERTY): https://dev.epicgames.com/documentation/en-us/unreal-engine/objects-in-unreal-engine
- UEnhancedInputComponent vs legacy UInputComponent: https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine
