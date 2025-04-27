#include "AmbientLightActor.h"
#include "Components/Light/AmbientLightComponent.h"
#include "Components/BillboardComponent.h"

AAmbientLight::AAmbientLight()
{
    AmbientLightComponent = AddComponent<UAmbientLightComponent>(TEXT("AmbientLightComponent"));

    BillboardComponent = AddComponent<UBillboardComponent>(TEXT("BillboardComponent"));
    BillboardComponent->SetTexture(L"Assets/Editor/Icon/AmbientLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;
}

AAmbientLight::~AAmbientLight()
{
}
