#include "BoxComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/ObjectFactory.h"


UBoxComponent::UBoxComponent()
{
    SetType(StaticClass()->GetName());

}

void UBoxComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UBoxComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}
