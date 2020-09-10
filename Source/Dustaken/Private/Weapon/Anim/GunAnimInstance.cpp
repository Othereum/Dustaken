// Copyright 2019 Seokjin Lee. All Rights Reserved.

#include "Weapon/Anim/GunAnimInstance.h"
#include "Player/FpsCharacter.h"
#include "Weapon/Gun.h"
#include "Weapon/WeaponComponent.h"

void UGunAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (const auto Gun = GetGun())
	{
		bEmpty = Gun->GetClip() == 0;
	}
}

AGun* UGunAnimInstance::GetGun() const
{
	return Cast<AGun>(GetSkelMeshComponent()->GetOwner());
}
