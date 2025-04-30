#include "CameraTypes.h"

#include "Math/Matrix.h"

bool FMinimalViewInfo::Equals(const FMinimalViewInfo& OtherInfo) const
{
    return 
        (Location == OtherInfo.Location) &&
        (Rotation == OtherInfo.Rotation) &&
        (FOV == OtherInfo.FOV) &&
        ((PerspectiveNearClipPlane == OtherInfo.PerspectiveNearClipPlane) || //either they are the same or both don't override
            (PerspectiveNearClipPlane <= 0.f && OtherInfo.PerspectiveNearClipPlane <= 0.f)) &&
        (AspectRatio == OtherInfo.AspectRatio) &&
        (bConstrainAspectRatio == OtherInfo.bConstrainAspectRatio) &&
        (bUseFieldOfViewForLOD == OtherInfo.bUseFieldOfViewForLOD) &&
        (ProjectionMode == OtherInfo.ProjectionMode) &&
        (OffCenterProjectionOffset == OtherInfo.OffCenterProjectionOffset);
}

void FMinimalViewInfo::BlendViewInfo(FMinimalViewInfo& OtherInfo, float OtherWeight)
{
    Location = FMath::Lerp(Location, OtherInfo.Location, OtherWeight);

    const FRotator DeltaAng = (OtherInfo.Rotation - Rotation).GetNormalized();
    Rotation = Rotation + DeltaAng * OtherWeight;

    FOV = FMath::Lerp(FOV, OtherInfo.FOV, OtherWeight);
    PerspectiveNearClipPlane = FMath::Lerp(PerspectiveNearClipPlane, OtherInfo.PerspectiveNearClipPlane, OtherWeight);
    OffCenterProjectionOffset = FMath::Lerp(OffCenterProjectionOffset, OtherInfo.OffCenterProjectionOffset, OtherWeight);

    AspectRatio = FMath::Lerp(AspectRatio, OtherInfo.AspectRatio, OtherWeight);
    bConstrainAspectRatio |= OtherInfo.bConstrainAspectRatio;
    bUseFieldOfViewForLOD |= OtherInfo.bUseFieldOfViewForLOD;
}

void FMinimalViewInfo::ApplyBlendWeight(const float& Weight)
{
    Location *= Weight;
    Rotation.Normalize();
    Rotation *= Weight;
    FOV *= Weight;
    PerspectiveNearClipPlane *= Weight;
    AspectRatio *= Weight;
    OffCenterProjectionOffset *= Weight;
}

void FMinimalViewInfo::AddWeightedViewInfo(const FMinimalViewInfo& OtherView, const float& Weight)
{
    FMinimalViewInfo OtherViewWeighted = OtherView;
    OtherViewWeighted.ApplyBlendWeight(Weight);

    Location += OtherViewWeighted.Location;
    Rotation += OtherViewWeighted.Rotation;
    FOV += OtherViewWeighted.FOV;
    PerspectiveNearClipPlane += OtherViewWeighted.PerspectiveNearClipPlane;
    AspectRatio += OtherViewWeighted.AspectRatio;
    OffCenterProjectionOffset += OtherViewWeighted.OffCenterProjectionOffset;

    bConstrainAspectRatio |= OtherViewWeighted.bConstrainAspectRatio;
    bUseFieldOfViewForLOD |= OtherViewWeighted.bUseFieldOfViewForLOD;
}

void FMinimalViewInfo::ApplyOverscan(float InOverscan, bool bScaleResolutionWithOverscan, bool bCropOverscan)
{
    if (!FMath::IsNearlyZero(InOverscan))
    {
        // 새로 적용할 오버스캔이 0 미만으로 내려가지 않도록 클램프
        const float ClampedOverscan = FMath::Max(InOverscan, -Overscan / (1 + Overscan));
		
        // 전체 오버스캔 양 업데이트
        // 수학적으로: 1 + TotalOverscan = (1 + 기존 Overscan) * (1 + InOverscan)
        Overscan = Overscan * (1.0f + ClampedOverscan) + ClampedOverscan;
		
        // 0.0은 오버스캔 없음, 1 더해서 프로젝션 스케일 계수로 사용
        const float OverscanScalar = 1.0f + ClampedOverscan;
		
        // 오버스캔은 뷰 프러스트럼을 확대하는 것과 동일
        // FOV를 조정하려면 Half-FOV의 탄젠트를 스케일해야 같은 효과를 얻음
        const float HalfFOVInRadians = FMath::DegreesToRadians(0.5f * FOV);
        const float OverscannedFOV = FMath::Atan(OverscanScalar * FMath::Tan(HalfFOVInRadians));
        FOV = 2.0f * FMath::RadiansToDegrees(OverscannedFOV);
        
        // 해상도 확장 비율 조정 (옵션에 따라)
        OverscanResolutionFraction *= bScaleResolutionWithOverscan ? OverscanScalar : 1.0f;
        // 크롭 비율 조정 (옵션에 따라)
        CropFraction *= bCropOverscan ? 1.0f / OverscanScalar : 1.0f;
    }
}

void FMinimalViewInfo::ClearOverscan()
{
    if (Overscan > 0.0f)
    {
        // 뷰 프러스트럼을 원래대로 복원하기 위해 역 오버스캔 적용
        // (1 + Overscan) * (1 + InverseOverscan) = 1 식에서 유도
        const float InverseOverscan = - Overscan / (1.0f + Overscan);
        ApplyOverscan(InverseOverscan);

        // 해상도 및 크롭 설정 초기화
        OverscanResolutionFraction = 1.0f;
        CropFraction = 1.0f;
    }
}
