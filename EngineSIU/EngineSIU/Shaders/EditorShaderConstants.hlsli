struct AABBData
{
    float3 Position;
    float Padding1;
    
    float3 Extent;
    float Padding2;
};
cbuffer ConstantBufferDebugAABB : register(b11)
{
    AABBData DataAABB[8];
}

struct SphereData
{
    float3 Position;
    float Radius;
};
cbuffer ConstantBufferDebugSphere : register(b11)
{
    SphereData DataSphere[8];
}

struct ConeData
{
    float3 ApexPosition;
    float InnerRadius;
    
    float OuterRadius;
    float3 Direction;
    
    float Height;
    float3 Padding;
};
cbuffer ConstantBufferDebugCone : register(b11)
{
    ConeData DataCone[100];
}

cbuffer ConstantBufferDebugGrid : register(b11)
{
    row_major matrix InverseViewProj;
}

cbuffer ConstantBufferDebugIcon : register(b11)
{
    float3 IconPosition;
    float IconScale;
}

cbuffer ConstantBufferDebugArrow : register(b11)
{
    float3 ArrowPosition;
    float ArrowScaleXYZ;
    float3 ArrowDirection;
    float ArrowScaleZ;
}

struct CapsuleData
{
    row_major float4x4 WorldMatrix;
    float3 CapsulePosition;
    float CapsuleRadius;
    float CapsuleHeight;
    float3 padding;
};

cbuffer ConstantBufferDebugCapsule : register(b11)
{
    CapsuleData DataCapsule[100];
}
