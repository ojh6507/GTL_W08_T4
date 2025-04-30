#include "PostProcessCompositingPass.h"

#include <array>

#include "RendererHelpers.h"
#include "UnrealClient.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Camera/PlayerCameraManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"
#include "Engine/Engine.h"

FPostProcessCompositingPass::FPostProcessCompositingPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
    , Sampler(nullptr)
{
}

FPostProcessCompositingPass::~FPostProcessCompositingPass()
{

}

void FPostProcessCompositingPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;

    ShaderManager->AddVertexShader(L"PostProcessCompositing", L"Shaders/PostProcessCompositingShader.hlsl", "mainVS");
    ShaderManager->AddPixelShader(L"PostProcessCompositing", L"Shaders/PostProcessCompositingShader.hlsl", "mainPS");

    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    Graphics->Device->CreateSamplerState(&SamplerDesc, &Sampler);


}

void FPostProcessCompositingPass::PrepareRender()
{
    PlayerCameraManager = nullptr;
    for (const auto iter : TObjectRange<APlayerCameraManager>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            PlayerCameraManager = iter;
            break;
        }
    }

}

void FPostProcessCompositingPass::Render(const std::shared_ptr<FEditorViewportClient>& ViewportClient)
{

    // --- 1. 유효성 검사 ---
    FViewportResource* ViewportResource = ViewportClient ? ViewportClient->GetViewportResource() : nullptr;
    if (!ViewportResource || !Graphics || !Graphics->DeviceContext || !ShaderManager || !Sampler || !BufferManager)
    {
        return;
    }

    const EResourceType TargetResourceType = EResourceType::ERT_PostProcessCompositing;
    FRenderTargetRHI* TargetRenderTargetRHI = ViewportResource->GetRenderTarget(TargetResourceType);
    FRenderTargetRHI* FogTextureRHI = ViewportResource->GetRenderTarget(EResourceType::ERT_PP_Fog);
    FRenderTargetRHI* SceneTextureRHI = ViewportResource->GetRenderTarget(EResourceType::ERT_Scene);

    if (!TargetRenderTargetRHI || !TargetRenderTargetRHI->Texture2D || !TargetRenderTargetRHI->RTV ||
        !FogTextureRHI || !FogTextureRHI->Texture2D || !FogTextureRHI->SRV || // <<< Fog Texture2D 추가 확인
        !SceneTextureRHI || !SceneTextureRHI->Texture2D || !SceneTextureRHI->SRV) // <<< Scene Texture2D 추가 확인
    {
        return;
    }
    ID3D11BlendState* AlphaBlendState = Graphics->AlphaBlendState;
    ID3D11BlendState* OpaqueBlendState = Graphics->OpaqueBlendState;

    // --- 2. 목표 크기 및 종횡비 계산 ---
    const float TargetWidth = static_cast<float>(TargetRenderTargetRHI->GetSizeX());
    const float TargetHeight = static_cast<float>(TargetRenderTargetRHI->GetSizeY());

    if (TargetWidth <= 0 || TargetHeight <= 0)
    {
        return;
    }
    const float TargetAspectRatio = TargetWidth / TargetHeight;
    const float SourceWidth = static_cast<float>(SceneTextureRHI->GetSizeX());
    const float SourceHeight = static_cast<float>(SceneTextureRHI->GetSizeY());

    if (SourceWidth <= 0 || SourceHeight <= 0)
    {
        return;
    }

    const float SourceAspectRatio = SourceWidth / SourceHeight;

    FCompositingParams ShaderParams;

    if (1)
    {
        float BaseRenderedWidth = TargetWidth;
        float BaseRenderedHeight = TargetHeight;
        if (SourceAspectRatio > TargetAspectRatio)
        {
            BaseRenderedHeight = TargetWidth / SourceAspectRatio;
        }
        else if (SourceAspectRatio < TargetAspectRatio)
        {
            BaseRenderedWidth = TargetHeight * SourceAspectRatio;
        }
        float FinalRenderedWidth = BaseRenderedWidth * ContentAreaScale;
        float FinalRenderedHeight = BaseRenderedHeight * ContentAreaScale;

        float FinalOffsetX = (TargetWidth - FinalRenderedWidth) * 0.5f;
        float FinalOffsetY = (TargetHeight - FinalRenderedHeight) * 0.5f;

        ShaderParams.LetterboxScale = FVector2D(FinalRenderedWidth / TargetWidth, FinalRenderedHeight / TargetHeight); // <<< 멤버 이름 변경

        float CenterX_Pixels = FinalOffsetX + FinalRenderedWidth * 0.5f;
        float CenterY_Pixels = FinalOffsetY + FinalRenderedHeight * 0.5f;

        ShaderParams.LetterboxOffset = FVector2D((CenterX_Pixels / TargetWidth) * 2.0f - 1.0f, 1.0f - (CenterY_Pixels / TargetHeight) * 2.0f); // <<< 멤버 이름 변경
    }
    else
    {
        ShaderParams.LetterboxScale = FVector2D(1.0f, 1.0f);
        ShaderParams.LetterboxOffset = FVector2D(0.0f, 0.0f);
    }


    if (PlayerCameraManager && PlayerCameraManager->bEnableFading)
    {
        ShaderParams.FadeAlpha = PlayerCameraManager->GetCurrentFadeAmount();
        ShaderParams.FadeColor = PlayerCameraManager->GetCurrentFadeColor();

        BufferManager->UpdateConstantBuffer<FCompositingParams>("FCompositingParams", ShaderParams);
        BufferManager->BindConstantBuffer(TEXT("FCompositingParams"), CompositingParamsConstantBufferSlot, EShaderStage::Pixel);

    }
    else
    {
        ShaderParams.FadeColor = FLinearColor(1, 1, 1, 1);
        ShaderParams.FadeAlpha = 1;
        BufferManager->UpdateConstantBuffer<FCompositingParams>("FCompositingParams", ShaderParams);
        BufferManager->BindConstantBuffer(TEXT("FCompositingParams"), CompositingParamsConstantBufferSlot, EShaderStage::Pixel);


    }

    // --- 6. 렌더링 상태 설정 ---
    ID3D11DeviceContext* Ctx = Graphics->DeviceContext;

    // 6a. 목표 렌더 타겟 클리어
    const float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Ctx->ClearRenderTargetView(TargetRenderTargetRHI->RTV, ClearColor);

    // 6b. 목표 렌더 타겟 및 뷰포트 설정
    Ctx->OMSetRenderTargets(1, &TargetRenderTargetRHI->RTV, nullptr);

    D3D11_VIEWPORT D3DViewport = {};
    D3DViewport.Width = TargetWidth;
    D3DViewport.Height = TargetHeight;
    D3DViewport.MinDepth = 0.0f;
    D3DViewport.MaxDepth = 1.0f;
    Ctx->RSSetViewports(1, &D3DViewport);

    // 6c. 셰이더 설정 (수정된 PostProcessCompositing 셰이더)
    ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(L"PostProcessCompositing");
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"PostProcessCompositing");
    if (!VertexShader || !PixelShader)
    {
        Ctx->OMSetRenderTargets(0, nullptr, nullptr); return;
    }

    Ctx->VSSetShader(VertexShader, nullptr, 0);
    Ctx->PSSetShader(PixelShader, nullptr, 0);

    // 6d. 입력 조립기 설정
    Ctx->IASetInputLayout(nullptr);
    Ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 6e. 픽셀 셰이더 리소스 설정 (Fog, Scene, Sampler)
    Ctx->PSSetShaderResources(FogTextureSlot, 1, &FogTextureRHI->SRV);
    Ctx->PSSetShaderResources(SceneTextureSlot, 1, &SceneTextureRHI->SRV);
    Ctx->PSSetSamplers(CompositingSamplerSlot, 1, &Sampler);

    // 6f. 블렌드 상태 설정 (알파 페이드 사용 시)
    float BlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    Ctx->OMSetBlendState(AlphaBlendState, BlendFactor, 0xffffffff);

    Ctx->RSSetState(Graphics->RasterizerSolidBack);

    // --- 7. 그리기 ---
    Ctx->Draw(6, 0);

    // --- 8. 마무리 및 리소스 해제 ---
    Ctx->OMSetBlendState(OpaqueBlendState, BlendFactor, 0xffffffff);

    Ctx->OMSetRenderTargets(0, nullptr, nullptr);

    // 사용한 픽셀 셰이더 리소스 슬롯 비우기
    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Ctx->PSSetShaderResources(FogTextureSlot, 1, NullSRV);
    Ctx->PSSetShaderResources(SceneTextureSlot, 1, NullSRV);

}
void FPostProcessCompositingPass::ClearRenderArr()
{

}

void FPostProcessCompositingPass::SetLetterboxingEnabled(bool bEnabled)
{

}

void FPostProcessCompositingPass::SetLetterboxScale(float InScale)
{

}
