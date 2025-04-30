#include "Components/SceneComponent.h"
#include "Math/Rotator.h"
#include "Math/JungleMath.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

USceneComponent::USceneComponent()
    : RelativeLocation(FVector(0.f, 0.f, 0.f))
    , RelativeRotation(FVector(0.f, 0.f, 0.f))
    , RelativeScale3D(FVector(1.f, 1.f, 1.f))
{
}

UObject* USceneComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->RelativeLocation = RelativeLocation;
    NewComponent->RelativeRotation = RelativeRotation;
    NewComponent->RelativeScale3D = RelativeScale3D;

    return NewComponent;
}

void USceneComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("RelativeLocation"), *RelativeLocation.ToString());
    OutProperties.Add(TEXT("RelativeRotation"), *RelativeRotation.ToString());
    OutProperties.Add(TEXT("RelativeScale3D"), *RelativeScale3D.ToString());
    
}

void USceneComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("RelativeLocation"));
    if (TempStr)
    {
        RelativeLocation.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("RelativeRotation"));
    if (TempStr)
    {
        RelativeRotation.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("RelativeScale3D"));
    if (TempStr)
    {
        RelativeScale3D.InitFromString(*TempStr);
    }
}

void USceneComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void USceneComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}


int USceneComponent::CheckRayIntersection(FVector& InRayOrigin, FVector& InRayDirection, float& pfNearHitDistance)
{
    // TODO: 나중에 지워도 될듯
    int nIntersections = 0;
    return nIntersections;
}

void USceneComponent::DestroyComponent()
{
    if (AttachParent)
    {
        // 자식 컴포넌트들의 부모를 자신에서 부모로 위임
        for (auto Child : AttachChildren)
        {
            // @todo 올바른 방식인지 확인 필요
            // @note 이미 SetupAttachment를 호출한 상태라면 다시 호출이 불가능함, 따라서 수동으로 부모를 설정
            //Child->SetupAttachment(AttachParent);
            AttachParent->AttachChildren.AddUnique(Child);
            Child->AttachParent = AttachParent;
        }

        AttachParent->AttachChildren.Remove(this);
        AttachParent = nullptr;
    }
    Super::DestroyComponent();
}

FVector USceneComponent::GetForwardVector()
{
	FVector Forward = FVector(1.f, 0.f, 0.0f);
	Forward = JungleMath::FVectorRotate(Forward, RelativeRotation);
	return Forward;
}

FVector USceneComponent::GetRightVector()
{
	FVector Right = FVector(0.f, 1.f, 0.0f);
	Right = JungleMath::FVectorRotate(Right, RelativeRotation);
	return Right;
}

FVector USceneComponent::GetUpVector()
{
	FVector Up = FVector(0.f, 0.f, 1.0f);
	Up = JungleMath::FVectorRotate(Up, RelativeRotation);
	return Up;
}


void USceneComponent::AddLocation(const FVector& InAddValue)
{
	RelativeLocation = RelativeLocation + InAddValue;

}

void USceneComponent::AddRotation(const FVector& InAddValue)
{
	RelativeRotation = RelativeRotation + InAddValue;

}

void USceneComponent::AddScale(const FVector& InAddValue)
{
	RelativeScale3D = RelativeScale3D + InAddValue;

}

void USceneComponent::AttachToComponent(USceneComponent* InParent)
{
    // 기존 부모와 연결을 끊기
    if (AttachParent)
    {
        AttachParent->AttachChildren.Remove(this);
    }

    // InParent도 nullptr이면 부모를 nullptr로 설정
    if (InParent == nullptr)
    {
        AttachParent = nullptr;
        return;
    }


    // 새로운 부모 설정
    AttachParent = InParent;

    // 부모의 자식 리스트에 추가
    if (!InParent->AttachChildren.Contains(this))
    {
        InParent->AttachChildren.Add(this);
    }
}

void USceneComponent::SetWorldLocation(const FVector& InNewLocation)
{
    if (AttachParent)
    {
        RelativeLocation = InNewLocation - AttachParent->GetWorldLocation();
    }
    else
    {
        RelativeLocation = InNewLocation;
    }
}

void USceneComponent::SetWorldRotation(const FRotator& InNewRotation)
{
    // 월드 회전을 쿼터니언으로 변환
    FQuat NewWorldQuat = InNewRotation.ToQuaternion();

    if (AttachParent)
    {
        // 상대 회전 계산: 부모의 역 쿼터니언과 곱함
        RelativeRotation = FRotator(AttachParent->GetWorldRotation().ToQuaternion().Inverse() * NewWorldQuat);
    }
    else
    {
        // 부모가 없으면 월드 회전이 곧 상대 회전
        RelativeRotation = InNewRotation;
    }
}

void USceneComponent::SetWorldScale3D(const FVector& NewScale)
{
    if (AttachParent)
    {
        RelativeScale3D = NewScale / AttachParent->GetWorldScale3D();
    }
    else
    {
        RelativeScale3D = NewScale;
    }
}

FVector USceneComponent::GetWorldLocation() const
{
    if (AttachParent)
    {
        return AttachParent->GetWorldLocation() + RelativeLocation;
    }
    return RelativeLocation;
}

FRotator USceneComponent::GetWorldRotation() const
{
    if (AttachParent)
    {
        return AttachParent->GetWorldRotation().ToQuaternion() * RelativeRotation.ToQuaternion();
    }
    return RelativeRotation;
}

FVector USceneComponent::GetWorldScale3D() const
{
    if (AttachParent)
    {
        return AttachParent->GetWorldScale3D() * RelativeScale3D;
    }
    return RelativeScale3D;
}

FMatrix USceneComponent::GetScaleMatrix() const
{
    FMatrix ScaleMat = FMatrix::GetScaleMatrix(RelativeScale3D);
    if (AttachParent)
    {
        FMatrix ParentScaleMat = AttachParent->GetScaleMatrix();
        ScaleMat = ScaleMat * ParentScaleMat;
    }
    return ScaleMat;
}

FMatrix USceneComponent::GetRotationMatrix() const
{
    FMatrix RotationMat = FMatrix::GetRotationMatrix(RelativeRotation);
    if (AttachParent)
    {
        FMatrix ParentRotationMat = AttachParent->GetRotationMatrix();
        RotationMat = RotationMat * ParentRotationMat;
    }
    return RotationMat;
}

FMatrix USceneComponent::GetTranslationMatrix() const
{
    FMatrix TranslationMat = FMatrix::GetTranslationMatrix(RelativeLocation);
    if (AttachParent)
    {
        FMatrix ParentTranslationMat = AttachParent->GetTranslationMatrix();
        TranslationMat = TranslationMat * ParentTranslationMat;
    }
    return TranslationMat;
}

FMatrix USceneComponent::GetWorldMatrix() const
{
    // 1) 부모 월드 매트릭스 가져오기 (없으면 단위행렬)
    FMatrix ParentWorld = AttachParent 
        ? AttachParent->GetWorldMatrix() 
        : FMatrix::Identity;
    
    FMatrix ScaleMat = FMatrix::GetScaleMatrix(RelativeScale3D);
    FMatrix RotationMat = FMatrix::GetRotationMatrix(RelativeRotation);
    FMatrix TranslationMat = FMatrix::GetTranslationMatrix(RelativeLocation);

    FMatrix LocalTRS = ScaleMat * RotationMat * TranslationMat;
    
    return LocalTRS * ParentWorld;
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    if (InParent != AttachParent)
    {
        if (InParent != this)
        {
            if (InParent == nullptr || !InParent->IsAttachedTo(this))
            {
                if (AttachParent == nullptr || !AttachParent->AttachChildren.Contains(this))
                {
                    AttachParent = InParent;

                    // TODO: .AddUnique의 실행 위치를 RegisterComponent로 바꾸거나 해야할 듯
                    AttachParent->AttachChildren.AddUnique(this);
                }
                else
                {
                    UE_LOG(ELogLevel::Error, TEXT("SetupAttachment cannot be used once a component has already had AttachTo used to connect it to a parent."));
                }
            }
            else
            {
                UE_LOG(ELogLevel::Error, TEXT("Setting up attachment would create a cycle."));
            }
        }
        else
        {
            UE_LOG(ELogLevel::Error, TEXT("Cannot attach a component to itself."));
        }
    }
}

bool USceneComponent::IsAttachedTo(const USceneComponent* TestComp) const
{
    if (TestComp != nullptr)
    {
        for (const USceneComponent* Comp = this->GetAttachParent(); Comp != nullptr; Comp = Comp->GetAttachParent())
        {
            if (TestComp == Comp)
            {
                return true;
            }
        }
    }
    return false;
}
