#pragma once
#include "IRenderPass.h"
#include "Delegates/Delegate.h"
#include "LevelEditor/SlateAppMessageHandler.h"

class APlayerCameraManager;

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

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;

    void SetLetterboxingEnabled(bool bEnabled);
    void SetLetterboxScale(float InScale);

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

    TArray<APlayerCameraManager*> PlayerCameraManagers;
};
