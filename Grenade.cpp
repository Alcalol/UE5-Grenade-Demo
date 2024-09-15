// Copyright 2024 Tony Sze

#include "Grenade.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h" 
#include "PhysicsEngine/RadialForceComponent.h"

// Sets default values
AGrenade::AGrenade()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Collider"));
	GrenadeMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Grenade Mesh Component"));
	GrenadeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Grenade Audio Component"));
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("Radial Force Component"));

	RootComponent = CapsuleComponent;
	GrenadeMeshComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
	GrenadeAudioComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RadialForceComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AGrenade::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("Grenade"));

	FuseTimeRemaining = FuseTime;

	if (GrenadeAudioComponent)
	{
		GrenadeAudioComponent->bAutoActivate = false;
	}

	if (CapsuleComponent)
	{
		CapsuleComponent->OnComponentHit.AddDynamic(this, &AGrenade::OnGrenadeHitObject);
	}
}

void AGrenade::PullPin(ACharacter* OwnerCharacter, bool PlayPinAudio)
{
	if (!bIsPinPulled)
	{
		if (PlayPinAudio)
		{
			PlaySound(PinPullSound);
		}

		GrenadeOwner = OwnerCharacter;
		bIsPinPulled = true;

		FTimerHandle FuseTimerHandle;
		GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AGrenade::Explode, FuseTime, false);
	}
}

void AGrenade::PopLever()
{
	PlaySound(LeverPopSound);
}

void AGrenade::ReleaseGrenade(FVector& AimDirection)
{
	if (CapsuleComponent)
	{
		CapsuleComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		EnablePhysics();
	}

	LaunchGrenade(AimDirection);

	if (TrailEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(TrailEffect, 
													 GrenadeMeshComponent, 
													 NAME_None, 
													 FVector::ZeroVector, 
													 FRotator::ZeroRotator, 
													 EAttachLocation::SnapToTargetIncludingScale, 
													 true, 
													 true);
	}
}

bool AGrenade::GetIsPinPulled()
{
	return bIsPinPulled;
}

bool AGrenade::GetIsReleased()
{
	return bIsReleased;
}

float AGrenade::GetFuseTimeRemaining()
{
	return FuseTimeRemaining;
}

ACharacter* AGrenade::GetOwnerCharacter()
{
	return GrenadeOwner;
}

void AGrenade::OnGrenadeHitObject(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (LastActorHit != OtherActor && !OtherActor->ActorHasTag(FName("Grenade")))
	{
		PlaySound(BounceSound);

		// Register the previous hit actor to avoid audio spam in case of fast/small bounces on the same surface
		LastActorHit = OtherActor;
	}
}

void AGrenade::EnablePhysics()
{
	if (CapsuleComponent)
	{
		CapsuleComponent->SetSimulatePhysics(true);
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void AGrenade::DisablePhysics()
{
	if (CapsuleComponent)
	{
		CapsuleComponent->SetSimulatePhysics(false);
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AGrenade::LaunchGrenade(FVector& AimDirection)
{
	if (CapsuleComponent)
	{
		CapsuleComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		CapsuleComponent->SetAllPhysicsLinearVelocity(AimDirection * ThrowForceMultiplier);
	}

	bIsReleased = true;
}

void AGrenade::Explode()
{
	if (GrenadeMeshComponent)
	{
		GrenadeMeshComponent->SetVisibility(false, true);
	}

	DisablePhysics();

	PlaySound(ExplosionSound);

	FVector ActorLocation = GetActorLocation();

	if (ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffect, ActorLocation, FRotator::ZeroRotator);
	}

	FireImpulse();

	PerformRadialDamage(ActorLocation);

	DrawExplosionDecal(ActorLocation);

	SpawnChildGrenades();

	TimedCleanup(3.0f);
}

// Impulse parameters set in Radial Force Component in editor, separate from grenade damage parameters
void AGrenade::FireImpulse()
{
	if (RadialForceComponent)
	{
		RadialForceComponent->FireImpulse();
	}
}

void AGrenade::PerformRadialDamage(FVector& Origin)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Init(this, 1);

	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), 
												   MaximumDamage, 
												   MinimumDamage, 
												   Origin, 
												   DamageInnerRadius, 
												   DamageOuterRadius, 
												   1.0f, 
												   UDamageType::StaticClass(), 
												   ActorsToIgnore, 
												   this, 
												   GrenadeOwner->Controller, 
												   ECollisionChannel::ECC_Visibility);
}

void AGrenade::DrawExplosionDecal(FVector& StartPoint)
{
	if (ExplosionDecal)
	{
		// Set EndPoint as low as possible to prevent decal drawing even if the grenade explodes in the air
		FVector EndPoint = FVector(StartPoint);
		EndPoint.Z -= 10.0f;

		// Face the decal in the correct direction
		FRotator DecalRotation = FRotator::ZeroRotator;
		DecalRotation.Pitch = -90;
		DecalRotation.Yaw = FMath::FRandRange(0.0f, 359.9f);

		FHitResult HitResult;

		// Line trace straight downwards to find where the surface the grenade is sitting on
		GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility);

		if (HitResult.bBlockingHit)
		{
			UGameplayStatics::SpawnDecalAtLocation(GetWorld(), 
												   ExplosionDecal, 
												   ExplosionDecalSize, 
												   HitResult.ImpactPoint, 
												   DecalRotation, 
												   10.0f);
		}
	}
}

float AGrenade::GenerateRandomAttitudeValue()
{
	float AttitudeValue = FMath::FRandRange(MinAngleOffsetFromVertical, MaxAngleOffsetFromVertical);

	// Randomly flip generated attitude values to cover positive and negative angles when generating
	if (FMath::RandBool()) {
		AttitudeValue *= -1;
	}

	// Translate value to be relative to up direction
	AttitudeValue = 90 + AttitudeValue;

	return AttitudeValue;
}

void AGrenade::SpawnChildGrenades()
{
	if (bSpawnChildGrenades && ChildGrenadeCount > 0 && AChildGrenadeType) {
		for (int i = 0; i < ChildGrenadeCount; i++)
		{
			// Generate random orientation for child grenades to spawn in
			const FVector SpawnLocation = GetActorLocation();
			const FRotator SpawnRotation = FRotator(FMath::FRandRange(0.0, 359.9),
												    FMath::FRandRange(0.0, 359.9),
													FMath::FRandRange(0.0, 359.9));
			const FActorSpawnParameters SpawnParams;

			AGrenade* NewChildGrenade = GetWorld()->SpawnActor<AGrenade>(AChildGrenadeType, SpawnLocation, SpawnRotation, SpawnParams);

			FRotator ChildLaunchDirection = FRotator(GenerateRandomAttitudeValue(),
													 FMath::FRandRange(0, 359.0f),
													 GenerateRandomAttitudeValue());

			if (NewChildGrenade)
			{
				FVector AimDirection = UKismetMathLibrary::GetForwardVector(ChildLaunchDirection);

				NewChildGrenade->PullPin(GrenadeOwner, false);
				NewChildGrenade->ReleaseGrenade(AimDirection);
			}
		}
	}
}

void AGrenade::PlaySound(USoundCue* SoundCue)
{
	if (SoundCue)
	{
		GrenadeAudioComponent->SetSound(SoundCue);
		GrenadeAudioComponent->Play();
	}
}

// Destroy self with a delay so explosion sound and VFX can play
void AGrenade::TimedCleanup(float TimeToWait)
{
	FTimerHandle DestroyTimerHandle;
	FTimerDelegate DestroyDelegate;

	DestroyDelegate.BindLambda([this]
		{
			Destroy();
		});

	GetWorldTimerManager().SetTimer(DestroyTimerHandle, DestroyDelegate, 0.0f, false, TimeToWait);
}

