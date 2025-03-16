#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <EnhancedInputLibrary.h>
#include "Car.generated.h"

UCLASS()
class ARCADECARCONTROLLER_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputMappingContext* InputMapping;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* Accelerate;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* Brake;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* Steering;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* CameraControl;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CarBody;

	//Suspension
	UPROPERTY(EditAnywhere, Category = "Suspension")
	float SuspensionRest = 70.0f; //Rest length of suspension

	UPROPERTY(EditAnywhere, Category = "Suspension")
	float SuspensionStiffness = 50000.0f; //Higher = stiffer suspension

	UPROPERTY(EditAnywhere, Category = "Suspension")
	float SuspensionDamping = 1000.0f; //Higher = less bounce


	//Speed Controls
	UPROPERTY(EditAnywhere, Category = "Friction")
	float MaxSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Friction")
	float Acceleration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Friction")
	float FrictionStrength = 0.5f;


	//Steering Controls
	UPROPERTY(EditAnywhere, Category = "Steering")
	float steerStrength = 10.0f;


	//Wheels
	UPROPERTY(VisibleAnywhere, Category = "Locations")
	USceneComponent* WheelFL; //Front Left Wheel
	UPROPERTY(VisibleAnywhere, Category = "Locations")
	USceneComponent* WheelFR; //Front Right Wheel
	UPROPERTY(VisibleAnywhere, Category = "Locations")
	USceneComponent* WheelRL; //Rear Left Wheel
	UPROPERTY(VisibleAnywhere, Category = "Locations")
	USceneComponent* WheelRR; //Rear Right Wheel

	UPROPERTY(VisibleAnywhere, Category = "Grounded")
	bool isGrounded;


	//Helper function for suspension
	void ApplySuspensionForce(USceneComponent* WheelLocation, float DeltaTime);
	void ApplyAcceleration();
	void ApplyBrake();
	void ApplySteering(const FInputActionValue& Value);
	void ApplyFriction(float DeltaTime);
	void ApplyCameraControl(const FInputActionValue& Value);
};
