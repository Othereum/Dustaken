// Copyright 2019 Seokjin Lee. All Rights Reserved.

#pragma once 

#include "GameFramework/HUD.h"
#include "DustakenHUD.generated.h"

UCLASS()
class ADustakenHUD : public AHUD
{
	GENERATED_BODY()

public:
	ADustakenHUD();

protected:
	void DrawHUD() override;

private:
	class UTexture2D* CrosshairTex;
};

