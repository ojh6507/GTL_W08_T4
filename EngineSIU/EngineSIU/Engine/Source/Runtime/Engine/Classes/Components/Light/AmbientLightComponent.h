﻿#pragma once
#include "LightComponent.h"
#include "LightDefine.h"

class UAmbientLightComponent : public ULightComponentBase
{
    DECLARE_CLASS(UAmbientLightComponent, ULightComponentBase)

public:
    UAmbientLightComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    const FAmbientLightInfo& GetAmbientLightInfo() const;
    void SetAmbientLightInfo(const FAmbientLightInfo& InAmbient);

    FLinearColor GetLightColor() const;
    void SetLightColor(const FLinearColor& InColor);

private:
    FAmbientLightInfo AmbientLightInfo;
};
