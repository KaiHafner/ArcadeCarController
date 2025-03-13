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
        Input->BindAction(Brake, ETriggerEvent::Triggered, this, &ACar::ApplyBrake);
        Input->BindAction(Steering, ETriggerEvent::Triggered, this, &ACar::ApplySteering);
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
    if (isGrounded) 
    {
        FVector Velocity = CarBody->GetPhysicsLinearVelocity();
        float CurrentSpeed = Velocity.Size();

        if (CurrentSpeed < MaxSpeed)
        {
            //Set acceleration speed curve
            static float localAcceleration = FMath::FInterpTo(localAcceleration, 1.0f, GetWorld()->GetDeltaSeconds(), 0.5f);

            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, "Accelerating");

            FVector ForceDirection = GetActorForwardVector();
            float ForceMagnitude = 100000.0f;
            FVector ForceToApply = ForceDirection * ForceMagnitude * localAcceleration * CarBody->GetMass();

            CarBody->AddForce(ForceToApply);
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, "Max Speed Reached!");
        }
    }
}

//Temporary Reversing (Brakes for now)
void ACar::ApplyBrake()
{
    if (isGrounded) 
    {
        FVector Velocity = CarBody->GetPhysicsLinearVelocity();
        float CurrentSpeed = Velocity.Size();

        if (CurrentSpeed < MaxSpeed)
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, "Reversing");

            FVector ForceDirection = GetActorForwardVector();
            float ForceMagnitude = 1000000.0f;
            FVector ForceToApply = ForceDirection * ForceMagnitude;

            CarBody->AddForce(-ForceToApply);
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, "Max Reverse Speed Reached!");
        }
    }
}

void ACar::ApplySteering(const FInputActionValue& Value)
{
    if (isGrounded) 
    {
        float SteeringInput = Value.Get<float>();

        GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, FString::Printf(TEXT("Steering Input: %f"), SteeringInput));
        GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, "Steering");

        if (FMath::Abs(SteeringInput) > 0.1f)
        {
            FVector Torque = FVector(0.0f, 0.0f, SteeringInput * steerStrength);
            CarBody->AddTorqueInRadians(Torque, NAME_None, true);
        }
    }
}



