#pragma once
#include "UShapeComponent.h"

class USphereComponent : public UShapeComponent
{
    DECLARE_CLASS(USphereComponent, UShapeComponent)

public:
    USphereComponent();

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
};
