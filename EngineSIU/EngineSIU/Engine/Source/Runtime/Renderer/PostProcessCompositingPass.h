#pragma once
#include "IRenderPass.h"
#include "LevelEditor/SlateAppMessageHandler.h"

struct FCompositingParams
{
    FVector2D LetterboxScale;
    FVector2D LetterboxOffset;
    float FadeAlpha;
    float Padding;
};
class FPostProcessCompositingPass : public IRenderPass
{
public:
    FPostProcessCompositingPass();
    virtual ~FPostProcessCompositingPass();
    
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    
    virtual void PrepareRender() override;

    void Tick(float DeltaTime);

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;
    
    void SetLetterboxingEnabled(bool bEnabled);
    void SetLetterboxScale(float InScale);

    void SetFadeFactor(float InFactor);

    void StartFadeIn(float Duration);
    void StartFadeOut(float Duration);

    void UpdateFade(float DeltaTime);

private:
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    ID3D11SamplerState* Sampler;


    bool bEnableLetterboxing;
    float ContentAreaScale = 1.8f;
   
    const UINT CompositingParamsConstantBufferSlot = 11;
    const UINT TextureSamplerSlot = 0;
    const UINT FogTextureSlot = 103; //  슬롯 번호 사용 (t103)
    const UINT SceneTextureSlot = 100; // 슬롯 번호 사용 (t100)
    const UINT CompositingSamplerSlot = 0;  // 슬롯 번호 사용 (s0)

    bool bIsFading;             // 페이드 진행 중 여부
    float CurrentFadeAlpha = 0;     // 현재 프레임의 알파 값
    float StartFadeAlpha;       // 페이드 시작 시점의 알파 값
    float TargetFadeAlpha;      // 목표 알파 값 (0.0 또는 1.0)
    float FadeDuration;         // 페이드 총 소요 시간
    float TimeSinceFadeStart;   // 페이드 시작 후 경과 시간

};
