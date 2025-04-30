
#include "ShaderRegisters.hlsl"
cbuffer PerFrame : register(b2)
{
    float2 ScreenSize; // 예: (1920, 1080)
    float2 QuadPixelSize; // 예: (32, 32)
    float VerticalOffset; // 예: 50 (중앙에서 아래로 내림)
};

struct VS_Input 
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

PS_Input main(VS_Input Input)
{
    PS_Input Output;

    // 화면 중앙 기준 픽셀 위치
    float2 centerPixel = ScreenSize * 0.5f + float2(0.0f, -500);

    // Quad Corner → 실제 픽셀 위치
    float2 pixelPos = centerPixel + (Input.Position.rg - 0.5f) * 32;

    // 픽셀 좌표 → NDC 변환
    float2 ndc = pixelPos / ScreenSize * 2.0f - 1.0f;
    //ndc.y *= -1.0f;

    Output.Position = float4(ndc, 0.f, 1.0f); // z = 0.5f, w = 1.0f

    Output.UV = Input.UV;
    return Output;
}
