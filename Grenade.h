// Copyright 2024 Tony Sze

#pragma once

#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

class ACharacter;
class UPrimitiveComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;
class UAudioComponent;
class URadialForceComponent;
class UNiagaraSystem;
class USoundCue;
class UMaterialInstance;

UCLASS()
class GRENADEDEMO_API AGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	AGrenade();

	UFUNCTION(BlueprintCallable)
	void PullPin(ACharacter* OwnerCharacter, bool PlayPinAudio);

	UFUNCTION(BlueprintCallable)
	void PopLever();

	UFUNCTION(BlueprintCallable)
	void ReleaseGrenade(FVector& AimDirection);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	bool GetIsPinPulled();

	UFUNCTION(BlueprintCallable)
	bool GetIsReleased();

	UFUNCTION(BlueprintCallable)
	float GetFuseTimeRemaining();

	UFUNCTION(BlueprintCallable)
	ACharacter* GetOwnerCharacter();

	UFUNCTION(BlueprintCallable)
	void OnGrenadeHitObject(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
#pragma region GrenadeSettings

	/***** Variables that are exposed to blueprints subclasses to configure grenade behaviour *****/

	UPROPERTY(EditDefaultsOnly)
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* GrenadeMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	UAudioComponent* GrenadeAudioComponent;

	UPROPERTY(EditDefaultsOnly)
	URadialForceComponent* RadialForceComponent;

	// Multiplier applied to the force of the grenade on release
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float ThrowForceMultiplier = 1700;	

	// How long until the grenade explodes in seconds
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float FuseTime = 5;					

	// The radius where players would receive the maximum damage
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float DamageInnerRadius = 120;		

	// The radius of the explosion in meters
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float DamageOuterRadius = 300;		

	// The maximum damage at the center of the explosion
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float MaximumDamage = 40.0;			

	// The minimum damage at the edge of the explosion
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float MinimumDamage = 1.0;			

	// Exponent for calculating damage falloff depending on distance
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float FalloffExponent = 1.0f;		

	// The Niagara explosion effect to play on detonation, set to None for no effect
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	UNiagaraSystem* ExplosionEffect;

	// The sound cue to use when the pin is pulled
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	USoundCue* PinPullSound;

	// The sound cue to use when the lever is popped
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	USoundCue* LeverPopSound;

	// The sound cue to use when the grenade bounces
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	USoundCue* BounceSound;

	// The sound cue to use when the explosion occurs
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	USoundCue* ExplosionSound;

	// The damage decal to leave on the floor where the explosion occurs
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	UMaterialInstance* ExplosionDecal;

	// The size of the explosion damage decal to leave
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	FVector ExplosionDecalSize = FVector(100.0f, 100.0f, 100.0f);

	// The Niagara trail effect to use, set to none for no trail
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	UNiagaraSystem* TrailEffect;

	// Should the explosion generate child grenade(s) that go in random directions?
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	bool bSpawnChildGrenades = false;	

	// How many child grenade(s) should be spawned on explosion
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings|Child Grenades", meta = (EditCondition = "bSpawnChildGrenades"))
	int ChildGrenadeCount = 0;			

	// Minimum angle offset from up direction in degrees that the child grenades can be launched
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings|Child Grenades", meta = (EditCondition = "bSpawnChildGrenades"))
	float MinAngleOffsetFromVertical = 10.0f;

	// Maximum angle offset from up direction in degrees that the child grenades can be launched
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings|Child Grenades", meta = (EditCondition = "bSpawnChildGrenades"))
	float MaxAngleOffsetFromVertical = 30.0f;

	//The grenade type the child grenades should be
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings|Child Grenades", meta = (EditCondition = "bSpawnChildGrenades"))
	TSubclassOf<AGrenade> AChildGrenadeType;	

#pragma endregion GrenadeSettings

	UPROPERTY()
	ACharacter* GrenadeOwner;

	UPROPERTY()
	AActor* LastActorHit;

	bool bIsPinPulled = false;

	bool bIsReleased = false;

	float FuseTimeRemaining = 0;

	void EnablePhysics();

	void DisablePhysics();

	void LaunchGrenade(FVector& AimDirection);

	void Explode();

	void FireImpulse();

	void PerformRadialDamage(FVector& Origin);

	void DrawExplosionDecal(FVector& StartPoint);

	void PlaySound(USoundCue* SoundCue);

	float GenerateRandomAttitudeValue();

	void SpawnChildGrenades();

	void TimedCleanup(float TimeToWait);
};
