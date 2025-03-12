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

    ApplySuspensionForce(WheelFL, DeltaTime);
    ApplySuspensionForce(WheelFR, DeltaTime);
    ApplySuspensionForce(WheelRL, DeltaTime);
    ApplySuspensionForce(WheelRR, DeltaTime);

}

void ACar::ApplySuspensionForce(USceneComponent* WheelLocation, float DeltaTime)
{
    if (!WheelLocation) return;

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    FVector RayStart = WheelLocation->GetComponentLocation(); 
    FVector RayDirection = -WheelLocation->GetUpVector();
    FVector RayEnd = RayStart + (RayDirection * SuspensionRest);

    //Perform raycast
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility, QueryParams);

    if (bHit)
    {
        isGrounded = true;

        float Compression = SuspensionRest - HitResult.Distance; //Suspension compression
        float SpringForce = SuspensionStiffness * Compression; //Hookes Law
        float Velocity = FVector::DotProduct(GetVelocity(), WheelLocation->GetUpVector()); //Vertical velocity relative to wheel
        float DampingForce = SuspensionDamping * Velocity;

        FVector Force = WheelLocation->GetUpVector() * (SpringForce - DampingForce); //Apply force along the wheels up direction

        CarBody->AddForceAtLocation(Force, WheelLocation->GetComponentLocation());

        DrawDebugLine(GetWorld(), RayStart, HitResult.Location, FColor::Green, false, 0.1f, 0, 2);
        DrawDebugPoint(GetWorld(), HitResult.Location, 5, FColor::Red, false, 0.1f);

        DrawDebugDirectionalArrow(GetWorld(), WheelLocation->GetComponentLocation(),
            WheelLocation->GetComponentLocation() + (Force * 0.001f),
            100.0f, FColor::Blue, false, 0.1f, 0, 3.0f);
    }
    else
    {
        isGrounded = false;
        DrawDebugLine(GetWorld(), RayStart, RayEnd, FColor::Red, false, 0.1f, 0, 2);
    }
}

void ACar::ApplyAcceleration()
{
    FVector Velocity = CarBody->GetPhysicsLinearVelocity();
    float CurrentSpeed = Velocity.Size();

    if (CurrentSpeed < MaxSpeed)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "Accelerating");

        FVector ForceDirection = GetActorForwardVector();
        float ForceMagnitude = 1000000.0f;
        FVector ForceToApply = ForceDirection * ForceMagnitude;

        CarBody->AddForce(ForceToApply);
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, "Max Speed Reached!");
    }
}



