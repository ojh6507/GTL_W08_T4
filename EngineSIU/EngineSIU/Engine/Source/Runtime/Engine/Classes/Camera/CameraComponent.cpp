#include "CameraComponent.h"

#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"

UCameraComponent::~UCameraComponent()
{
}

void UCameraComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
}

void UCameraComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void UCameraComponent::OnComponentDestroyed()
{
    Super::OnComponentDestroyed();
}

void UCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

UObject* UCameraComponent::Duplicate(UObject* InOuter)
{
    UCameraComponent* NewComponent = Cast<UCameraComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->ViewFOV = ViewFOV;
        NewComponent->NearClip = NearClip;
        NewComponent->FarClip = FarClip;

        NewComponent->View = View;
        NewComponent->Projection = Projection;
    }
    return NewComponent;
}

void UCameraComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("ViewFOV"), FString::Printf(TEXT("%f"), ViewFOV));
    OutProperties.Add(TEXT("NearClip"), FString::Printf(TEXT("%f"), NearClip));
    OutProperties.Add(TEXT("FarClip"), FString::Printf(TEXT("%f"), FarClip));
}

void UCameraComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("ViewFOV"));
    if (TempStr)
    {
        ViewFOV = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("NearClip"));
    if (TempStr)
    {
        NearClip = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FarClip"));
    if (TempStr)
    {
        FarClip = FString::ToFloat(*TempStr);
    }
}

void UCameraComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UCameraComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void UCameraComponent::DestroyComponent()
{
    Super::DestroyComponent();
}

void UCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
    DesiredView.PreviousViewLocation = GetWorldLocation();
    DesiredView.PreviousViewRotation = GetWorldRotation();
    DesiredView.PreviousViewScale = GetWorldScale3D();

    // TODO : Pawn 생기면
    //if (bUsePawnControlRotation)
    //{
        //const APawn* OwningPawn = Cast<APawn>(GetOwner());
        //const AController* OwningController = OwningPawn ? OwningPawn->GetController() : nullptr;
        //if (OwningController && OwningController->IsLocalPlayerController())
        //{
            //const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
            //if (!PawnViewRotation.Equals(GetComponentRotation()))
            //{
                //SetWorldRotation(PawnViewRotation);
            //}
        //}
    //}
    
    DesiredView.Location = GetWorldLocation();
    DesiredView.Rotation = GetWorldRotation();
        
    DesiredView.FOV = bUseAdditiveOffset ? (ViewFOV + AdditiveFOVOffset) : ViewFOV;
    DesiredView.AspectRatio = AspectRatio;
    DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
    DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
    DesiredView.ProjectionMode = ProjectionMode;
    
    DesiredView.AutoPlaneShift = AutoPlaneShift;
	
    DesiredView.ApplyOverscan(Overscan, bScaleResolutionWithOverscan, bCropOverscan);

    if (bOverrideAspectRatioAxisConstraint)
    {
        DesiredView.AspectRatioAxisConstraint = AspectRatioAxisConstraint;
    }

    // See if the CameraActor wants to override the PostProcess settings used.
    DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
    if (PostProcessBlendWeight > 0.0f)
    {
        //DesiredView.PostProcessSettings = PostProcessSettings;
    }
}

void UCameraComponent::UpdateViewMatrix()
{
    View = JungleMath::CreateViewMatrix(GetWorldLocation(),
          GetWorldLocation() + GetForwardVector(),
           FVector{ 0.0f,0.0f, 1.0f }
       );
}

void UCameraComponent::UpdateProjectionMatrix()
{
    const std::shared_ptr<FEditorViewportClient> ActiveViewporClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
    const float AspectRatio = ActiveViewporClient->GetD3DViewport().Width / ActiveViewporClient->GetD3DViewport().Height;
    Projection = JungleMath::CreateProjectionMatrix(
        FMath::DegreesToRadians(ViewFOV),
        AspectRatio,
        NearClip,
        FarClip
    );
}
