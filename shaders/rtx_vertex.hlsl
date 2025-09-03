// RTX-Enhanced Vertex Shader
// Provides additional vertex data for ray tracing calculations

cbuffer PerFrameConstants : register(b0)
{
    matrix WorldViewProj;
    matrix World;
    matrix WorldInverseTranspose;
    float3 CameraPosition;
    float Time;
    matrix PrevWorldViewProj;  // For motion vectors
};

cbuffer RTXConstants : register(b1)
{
    float RTXIntensity;
    float GlobalIlluminationStrength;
    float ReflectionStrength;
    int BounceCount;
};

struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : WORLD_POSITION;
    float3 WorldNormal : WORLD_NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float4 ScreenPosition : SCREEN_POSITION;
    float4 PrevScreenPosition : PREV_SCREEN_POSITION;
    float3 ViewDirection : VIEW_DIRECTION;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    // Transform position
    float4 worldPos = mul(float4(input.Position, 1.0), World);
    output.WorldPosition = worldPos.xyz;
    output.Position = mul(worldPos, WorldViewProj);
    
    // Transform normals and tangent space
    output.WorldNormal = normalize(mul(input.Normal, (float3x3)WorldInverseTranspose));
    output.Tangent = normalize(mul(input.Tangent, (float3x3)World));
    output.Binormal = normalize(mul(input.Binormal, (float3x3)World));
    
    // Texture coordinates
    output.TexCoord = input.TexCoord;
    
    // Screen space positions for temporal effects
    output.ScreenPosition = output.Position;
    output.PrevScreenPosition = mul(worldPos, PrevWorldViewProj);
    
    // View direction for reflections
    output.ViewDirection = normalize(CameraPosition - worldPos.xyz);
    
    return output;
}
