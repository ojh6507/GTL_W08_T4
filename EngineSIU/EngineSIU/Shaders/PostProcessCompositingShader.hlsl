Texture2D FogTexture : register(t103);
Texture2D SceneTexture : register(t100);

// PostProcessing 추가 시 Texture 추가 (EShaderSRVSlot)

SamplerState CompositingSampler : register(s0);

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

float4 mainPS(PS_Input input) : SV_Target
{
    float2 uv = input.UV;
    float2 coord = (floor(input.UV * 500) + 0.5) / 500;
    float4 c = SceneTexture.Sample(CompositingSampler, uv);
    float4 final = c; //* float4(0.01f, 0.01, 0.01, 0.5);
    
    
    float3 col = SceneTexture.Sample(CompositingSampler, coord).rgb;

    col = floor(col * 5) / 5;
    
    return final; // float4(col, 1);

}
