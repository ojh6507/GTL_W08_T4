Texture2D FogTexture : register(t103);
Texture2D SceneTexture : register(t100);

// PostProcessing 추가 시 Texture 추가 (EShaderSRVSlot)

SamplerState CompositingSampler : register(s0);

cbuffer CompositingParams : register(b11)
{
    float2 LetterboxScale; // 레터박스 영역 스케일
    float2 LetterboxOffset; // 레터박스 영역 중심 오프셋 (NDC)
    float4 FadeColor;
    float FadeAlpha; // 페이드 알파 (0.0 = 투명, 1.0 = 불투명)
};
struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

PS_Input mainVS(uint VertexID : SV_VertexID)
{
    PS_Input Output;

    float2 QuadPositions[6] = {
        float2(-1,  1),  // Top Left
        float2(1,  1),  // Top Right
        float2(-1, -1),  // Bottom Left
        float2(1,  1),  // Top Right
        float2(1, -1),  // Bottom Right
        float2(-1, -1)   // Bottom Left
    };

    float2 UVs[6] = {
        float2(0, 0), float2(1, 0), float2(0, 1),
        float2(1, 0), float2(1, 1), float2(0, 1)
    };

    Output.Position = float4(QuadPositions[VertexID], 0, 1);
    Output.UV = UVs[VertexID];

    return Output;
}

// 픽셀 셰이더 (레터박스, 페이드 로직 추가)
float4 mainPS(PS_Input input) : SV_Target
{
    // --- 레터박스 영역 계산 ---
    float2 currentNDC = float2(input.UV.x * 2.0 - 1.0, 1.0 - input.UV.y * 2.0);

    float2 halfSize = LetterboxScale * 0.5;
    float2 minNDC = LetterboxOffset - halfSize; // 중심 - 절반 크기 = 최소 좌표
    float2 maxNDC = LetterboxOffset + halfSize; // 중심 + 절반 크기 = 최대 좌표

    float insideX = step(minNDC.x, currentNDC.x) * step(currentNDC.x, maxNDC.x);
    float insideY = step(minNDC.y, currentNDC.y) * step(currentNDC.y, maxNDC.y);
    float isInsideLetterbox = insideX * insideY;

    // --- 색상 계산 ---
    float4 outputColor = float4(0.0, 0.0, 0.0, 1.0);
    
    if (isInsideLetterbox > 0.5) // 부동소수점 오차 감안
    {
        // Fog 및 Scene 텍스처 샘플링
        float4 fogColor = FogTexture.Sample(CompositingSampler, input.UV);
        float4 sceneColor = SceneTexture.Sample(CompositingSampler, input.UV);

        // --- 실제 합성 로직 (예시: 간단한 알파 블렌딩) ---
        // 필요에 따라 더 복잡한 합성 로직으로 대체 가능
        float3 compositedRGB = lerp(sceneColor.rgb, fogColor.rgb, fogColor.a);
        float compositedAlpha = sceneColor.a; // 최종 알파는 Scene의 알파를 따르거나 1.0 사용

        outputColor = float4(compositedRGB, compositedAlpha);
    }

    outputColor.rgb = lerp(FadeColor.rgb, outputColor.rgb, FadeAlpha);

    return outputColor;
}
