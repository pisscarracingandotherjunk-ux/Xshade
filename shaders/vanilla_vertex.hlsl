// Vanilla Vertex Shader - Standard Roblox-style rendering

cbuffer PerFrameConstants : register(b0)
{
    matrix WorldViewProj;
    matrix World;
    matrix WorldInverseTranspose;
    float3 CameraPosition;
    float Time;
};

struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : WORLD_POSITION;
    float3 WorldNormal : WORLD_NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 ViewDirection : VIEW_DIRECTION;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    // Transform position
    float4 worldPos = mul(float4(input.Position, 1.0), World);
    output.WorldPosition = worldPos.xyz;
    output.Position = mul(worldPos, WorldViewProj);
    
    // Transform normal
    output.WorldNormal = normalize(mul(input.Normal, (float3x3)WorldInverseTranspose));
    
    // Texture coordinates
    output.TexCoord = input.TexCoord;
    
    // View direction
    output.ViewDirection = normalize(CameraPosition - worldPos.xyz);
    
    return output;
}
