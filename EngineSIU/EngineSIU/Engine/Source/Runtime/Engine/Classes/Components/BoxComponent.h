#pragma once
#include "UShapeComponent.h"

class UBoxComponent : public UShapeComponent
{
    DECLARE_CLASS(UBoxComponent, UShapeComponent)

public:
    UBoxComponent();

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
};
