// Fill out your copyright notice in the Description page of Project Settings.


#include "Car.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>


// Sets default values
ACar::ACar()
{
    PrimaryActorTick.bCanEverTick = true;

    //Set up the car body mesh
    CarBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarBody"));
    RootComponent = CarBody;

    //Initialize wheel components to nullptr
    WheelFL = nullptr;
    WheelFR = nullptr;
    WheelRL = nullptr;
    WheelRR = nullptr;
}

void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    //Add input mapping context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller)) {
        //Gets local player subsystem
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            //Add input context
            Subsystem->AddMappingContext(InputMapping, 0);
        }
    }

    if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        Input->BindAction(Accelerate, ETriggerEvent::Triggered, this, &ACar::ApplyAcceleration);
    }
}

void ACar::BeginPlay()
{
    Super::BeginPlay();

    //Directly reference the existing wheel components placed in the Blueprint
    WheelFL = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("FL_Wheel")));
    WheelFR = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("FR_Wheel")));
    WheelRL = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("RL_Wheel")));
    WheelRR = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("RR_Wheel")));
}

void ACar::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ApplySuspensionForce(WheelFL->GetComponentLocation(), DeltaTime);
    ApplySuspensionForce(WheelFR->GetComponentLocation(), DeltaTime);
    ApplySuspensionForce(WheelRL->GetComponentLocation(), DeltaTime);
    ApplySuspensionForce(WheelRR->GetComponentLocation(), DeltaTime);

}

void ACar::ApplySuspensionForce(FVector WheelLocation, float DeltaTime)
{
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); //Ignore the car itself

    FVector RayStart = WheelLocation;
    FVector RayEnd = RayStart - FVector(0, 0, SuspensionRest); //Always cast straight down in world space

    //Perform raycast
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility, QueryParams);

    if (bHit)
    {
        isGrounded = true;

        float Compression = SuspensionRest - HitResult.Distance; //Suspension compression
        float SpringForce = SuspensionStiffness * Compression; //hookes Law
        float Velocity = FVector::DotProduct(GetVelocity(), FVector(0, 0, 1)); //Vertical velocity only
        float DampingForce = SuspensionDamping * Velocity;

        FVector Force = FVector(0, 0, SpringForce - DampingForce); //apply force only upwards

        CarBody->AddForceAtLocation(Force, WheelLocation);

        //Debug draw - Raycast
        DrawDebugLine(GetWorld(), RayStart, HitResult.Location, FColor::Green, false, 0.1f, 0, 2);
        DrawDebugPoint(GetWorld(), HitResult.Location, 5, FColor::Red, false, 0.1f);

        //Debug arrow showing force direction
        DrawDebugDirectionalArrow(GetWorld(), WheelLocation, WheelLocation + (Force * 0.001f), 100.0f, FColor::Blue, false, 0.1f, 0, 3.0f);
    }
    else
    {
        isGrounded = false;
        //draw a failed line in red
        DrawDebugLine(GetWorld(), RayStart, RayEnd, FColor::Red, false, 0.1f, 0, 2);
    }
}

void ACar::ApplyAcceleration()
{
    GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "Pressed");

    FVector ForceDirection = GetActorForwardVector(); // Get the forward direction of the car
    float ForceMagnitude = 10000.0f; // Adjust this value based on your needs
    FVector ForceToApply = ForceDirection * ForceMagnitude;
    CarBody->AddForce(ForceToApply);
}



