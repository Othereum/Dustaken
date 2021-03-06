// Copyright 2019 Seokjin Lee. All Rights Reserved.

#include "Weapon/Weapon.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

#include "Player/FpsCharacter.h"
#include "Weapon/WeaponComponent.h"

AWeapon::AWeapon()
	:Mesh{ CreateDefaultSubobject<USkeletalMeshComponent>("Mesh") }
{
	bReplicates = true;
	bAlwaysRelevant = true;

	PrimaryActorTick.bCanEverTick = true;

	RootComponent = Mesh;
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

void AWeapon::Init()
{
	if (const auto Character = Cast<AFpsCharacter>(GetOwner()))
	{
		Owner = Character;
		SetRole(Owner->GetLocalRole());
		if (State == EWeaponState::NeverDeployed && Owner->GetWeaponComponent()->GetActiveWeapon() == this)
		{
			Deploy();
		}
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AWeapon::Init);
	}
}

void AWeapon::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (const auto Character = GetCharacter())
	{
		SetRole(Character->GetLocalRole());
	}
}

void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Mesh->SetVisibility(false);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, State);
}

void AWeapon::MulticastDeploy_Implementation()
{
	Deploy();
}

void AWeapon::MulticastHolster_Implementation(AWeapon* To)
{
	Holster(To);
}

bool AWeapon::IsVisible() const
{
	return Mesh->IsVisible();
}

void AWeapon::SetVisibility(const bool bNewVisibility) const
{
	Mesh->SetVisibility(bNewVisibility);
}

AFpsCharacter* AWeapon::GetCharacter() const
{
	return Owner;
}

void AWeapon::Deploy()
{
	PlayOwnerAnim(DeployAnim, DeployTime);
	if (HasAuthority()) State = EWeaponState::Deploying;
	Mesh->SetVisibility(true);
	if (HasAuthority()) GetWorldTimerManager().SetTimer(StateSetTimer, [this] { State = EWeaponState::Idle; }, DeployTime, false);
}

bool AWeapon::CanDeploy() const
{
	switch (State)
	{
	case EWeaponState::NeverDeployed:
	case EWeaponState::Unequipped:
		return true;
	default:
		return false;
	}
}

void AWeapon::Holster(AWeapon* To)
{
	if (HasAuthority()) State = EWeaponState::Holstering;
	PlayOwnerAnim(HolsterAnim, HolsterTime);
	const auto ToSlot = To->GetSlot();
	GetWorldTimerManager().SetTimer(StateSetTimer, [this, ToSlot]
	{
		Mesh->SetVisibility(false);
		if (HasAuthority())
		{
			State = EWeaponState::Unequipped;
			if (const auto Character = GetCharacter())
				Character->GetWeaponComponent()->SelectWeapon(ToSlot);
		}
	}, HolsterTime, false);
}

void AWeapon::PlayOwnerAnim(UAnimMontage* Anim, const float Time, const bool bConsiderBlendOutTime) const
{
	const auto Character = GetCharacter();
	if (Character && Anim)
	{
		auto AnimLength = Anim->SequenceLength * Anim->RateScale;
		if (bConsiderBlendOutTime) AnimLength -= Anim->BlendOut.GetBlendTime();
		if (AnimLength > 0.f)
		{
			Character->PlayAnimMontage(Anim, AnimLength / Time);
		}
	}
}

void AWeapon::PlayWepAnim(UAnimMontage* Anim) const
{
	if (const auto AnimInstance = Mesh->GetAnimInstance())
	{
		AnimInstance->Montage_Play(Anim);
	}
}

void AWeapon::StopWepAnim(const float BlendOutTime, UAnimMontage* Anim) const
{
	if (const auto AnimInstance = Mesh->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(BlendOutTime, Anim);
	}
}

bool AWeapon::CanHolster() const
{
	switch (State)
	{
	case EWeaponState::Idle:
	case EWeaponState::Firing:
	case EWeaponState::Reloading:
		return true;
	default:
		return false;
	}
}
