// Copyright 2019 Seokjin Lee. All Rights Reserved.

#include "Weapon/Anim/AnimNotify_GrabMag.h"
#include "Weapon/Gun.h"

void UAnimNotify_GrabMag::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (const auto Gun = GetGun(MeshComp))
	{
		Gun->GrabMag();
	}
}
