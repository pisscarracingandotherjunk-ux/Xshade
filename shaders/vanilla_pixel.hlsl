// Vanilla Pixel Shader - Standard Roblox-style rendering

Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

cbuffer MaterialConstants : register(b0)
{
    float3 DiffuseColor;
    float3 SpecularColor;
    float SpecularPower;
    float3 EmissiveColor;
};

cbuffer LightingConstants : register(b1)
{
    float3 SunDirection;
    float SunIntensity;
    float3 SunColor;
    float3 AmbientColor;
    float AmbientIntensity;
};

struct PixelInput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : WORLD_POSITION;
    float3 WorldNormal : WORLD_NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 ViewDirection : VIEW_DIRECTION;
};

float4 main(PixelInput input) : SV_TARGET
{
    // Sample diffuse texture
    float4 diffuseSample = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float3 albedo = diffuseSample.rgb * DiffuseColor;
    
    // Normalize inputs
    float3 normal = normalize(input.WorldNormal);
    float3 lightDir = normalize(-SunDirection);
    float3 viewDir = normalize(input.ViewDirection);
    
    // Lambert diffuse lighting
    float NdotL = saturate(dot(normal, lightDir));
    float3 diffuse = albedo * SunColor * SunIntensity * NdotL;
    
    // Blinn-Phong specular
    float3 halfDir = normalize(lightDir + viewDir);
    float NdotH = saturate(dot(normal, halfDir));
    float3 specular = SpecularColor * pow(NdotH, SpecularPower) * NdotL;
    
    // Ambient lighting
    float3 ambient = albedo * AmbientColor * AmbientIntensity;
    
    // Final color
    float3 finalColor = diffuse + specular + ambient + EmissiveColor;
    
    // Simple gamma correction
    finalColor = pow(finalColor, 1.0 / 2.2);
    
    return float4(finalColor, diffuseSample.a);
}
