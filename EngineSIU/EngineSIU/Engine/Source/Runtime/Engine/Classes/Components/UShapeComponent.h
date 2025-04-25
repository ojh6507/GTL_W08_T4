#pragma once
#include "PrimitiveComponent.h"

class UShapeComponent : public UPrimitiveComponent
{
public:
    FColor ShapeColor;
    bool bDrawOnlyIfSelected;
};
