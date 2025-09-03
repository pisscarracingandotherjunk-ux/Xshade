// RTX-Enhanced Pixel Shader
// Implements global illumination, reflections, and enhanced lighting

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D SpecularTexture : register(t2);
Texture2D EnvironmentMap : register(t3);
Texture2D PreviousFrame : register(t4);
Texture2D DepthBuffer : register(t5);
Texture2D GIBuffer : register(t6);

SamplerState LinearSampler : register(s0);
SamplerState PointSampler : register(s1);
SamplerState EnvironmentSampler : register(s2);

cbuffer MaterialConstants : register(b0)
{
    float3 DiffuseColor;
    float Metallic;
    float3 SpecularColor;
    float Roughness;
    float3 EmissiveColor;
    float NormalIntensity;
};

cbuffer RTXSettings : register(b1)
{
    float RTXIntensity;
    float GlobalIlluminationStrength;
    float ReflectionStrength;
    int BounceCount;
    float3 SunDirection;
    float SunIntensity;
    float3 SunColor;
    float AmbientIntensity;
    float ShadowSoftness;
    float DenoiseStrength;
};

cbuffer PerFrameConstants : register(b2)
{
    float3 CameraPosition;
    float Time;
    matrix ViewMatrix;
    matrix ProjMatrix;
    float2 ScreenSize;
    float2 TexelSize;
};

struct PixelInput
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

// Utility functions
float3 SampleEnvironmentMap(float3 direction)
{
    // Convert world direction to spherical coordinates for cubemap sampling
    float2 uv;
    uv.x = atan2(direction.z, direction.x) / (2.0 * 3.14159) + 0.5;
    uv.y = acos(direction.y) / 3.14159;
    return EnvironmentMap.Sample(EnvironmentSampler, uv).rgb;
}

float3 ComputeGlobalIllumination(float3 worldPos, float3 normal, float2 screenUV)
{
    float3 gi = float3(0, 0, 0);
    
    // Sample pre-computed GI buffer
    float3 indirectLight = GIBuffer.Sample(LinearSampler, screenUV).rgb;
    
    // Apply hemisphere sampling for additional bounces
    const int sampleCount = 8;
    for (int i = 0; i < sampleCount; i++)
    {
        float phi = 2.0 * 3.14159 * (float(i) / float(sampleCount));
        float cosTheta = sqrt((i + 1.0) / float(sampleCount + 1));
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
        
        float3 sampleDir = float3(
            sinTheta * cos(phi),
            cosTheta,
            sinTheta * sin(phi)
        );
        
        // Transform to world space
        float3 tangent = normalize(cross(normal, float3(0, 1, 0)));
        if (length(tangent) < 0.1)
            tangent = normalize(cross(normal, float3(1, 0, 0)));
        float3 bitangent = cross(normal, tangent);
        
        float3 worldSampleDir = tangent * sampleDir.x + normal * sampleDir.y + bitangent * sampleDir.z;
        
        // Sample environment
        float3 envColor = SampleEnvironmentMap(worldSampleDir);
        gi += envColor * cosTheta;
    }
    
    gi = gi / float(sampleCount) + indirectLight;
    return gi * GlobalIlluminationStrength;
}

float3 ComputeReflections(float3 worldPos, float3 normal, float3 viewDir, float roughness, float2 screenUV)
{
    float3 reflectDir = reflect(-viewDir, normal);
    
    // Screen space reflections
    float3 reflection = float3(0, 0, 0);
    
    // Trace in screen space
    float3 rayStart = worldPos;
    float3 rayDir = reflectDir;
    
    const int maxSteps = 32;
    const float stepSize = 0.1;
    
    for (int i = 0; i < maxSteps; i++)
    {
        float3 samplePos = rayStart + rayDir * (i * stepSize);
        
        // Project to screen space
        float4 clipPos = mul(float4(samplePos, 1.0), mul(ViewMatrix, ProjMatrix));
        float2 sampleUV = (clipPos.xy / clipPos.w) * 0.5 + 0.5;
        sampleUV.y = 1.0 - sampleUV.y; // Flip Y
        
        if (sampleUV.x >= 0 && sampleUV.x <= 1 && sampleUV.y >= 0 && sampleUV.y <= 1)
        {
            float depth = DepthBuffer.Sample(PointSampler, sampleUV).r;
            if (clipPos.z / clipPos.w <= depth + 0.001) // Hit surface
            {
                reflection = PreviousFrame.Sample(LinearSampler, sampleUV).rgb;
                break;
            }
        }
    }
    
    // Fallback to environment map
    if (length(reflection) < 0.01)
    {
        reflection = SampleEnvironmentMap(reflectDir);
    }
    
    // Fresnel effect
    float fresnel = pow(1.0 - saturate(dot(viewDir, normal)), 5.0);
    float reflectance = lerp(0.04, 1.0, fresnel);
    
    return reflection * reflectance * ReflectionStrength * (1.0 - roughness);
}

float ComputeRaytracedShadows(float3 worldPos, float3 lightDir)
{
    // Simplified raytraced shadow approximation
    float shadow = 1.0;
    
    const int samples = 4;
    const float radius = ShadowSoftness;
    
    for (int i = 0; i < samples; i++)
    {
        float angle = (float(i) / float(samples)) * 2.0 * 3.14159;
        float3 offset = float3(cos(angle), 0, sin(angle)) * radius;
        float3 sampleDir = normalize(lightDir + offset);
        
        // This would normally trace rays against geometry
        // For now, we'll use a simplified approximation
        shadow *= 0.8 + 0.2 * saturate(dot(sampleDir, lightDir));
    }
    
    return shadow;
}

float4 main(PixelInput input) : SV_TARGET
{
    // Sample textures
    float4 diffuseSample = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float4 normalSample = NormalTexture.Sample(LinearSampler, input.TexCoord);
    float4 specularSample = SpecularTexture.Sample(LinearSampler, input.TexCoord);
    
    // Unpack normal map
    float3 tangentNormal = normalSample.rgb * 2.0 - 1.0;
    tangentNormal.xy *= NormalIntensity;
    
    // Transform to world space
    float3x3 TBN = float3x3(
        normalize(input.Tangent),
        normalize(input.Binormal),
        normalize(input.WorldNormal)
    );
    float3 worldNormal = normalize(mul(tangentNormal, TBN));
    
    // Material properties
    float3 albedo = diffuseSample.rgb * DiffuseColor;
    float metallic = specularSample.b * Metallic;
    float roughness = specularSample.g * Roughness;
    float ao = specularSample.r;
    
    // View direction
    float3 viewDir = normalize(input.ViewDirection);
    
    // Screen UV for screen-space effects
    float2 screenUV = input.ScreenPosition.xy / input.ScreenPosition.w * 0.5 + 0.5;
    screenUV.y = 1.0 - screenUV.y;
    
    // RTX Features
    float3 finalColor = EmissiveColor;
    
    // Direct lighting
    float3 lightDir = normalize(-SunDirection);
    float NdotL = saturate(dot(worldNormal, lightDir));
    
    if (NdotL > 0.0)
    {
        // Raytraced shadows
        float shadow = ComputeRaytracedShadows(input.WorldPosition, lightDir);
        
        // BRDF lighting
        float3 halfDir = normalize(lightDir + viewDir);
        float NdotH = saturate(dot(worldNormal, halfDir));
        float VdotH = saturate(dot(viewDir, halfDir));
        
        // Fresnel
        float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
        float3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
        
        // Distribution
        float alpha = roughness * roughness;
        float alpha2 = alpha * alpha;
        float denom = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
        float D = alpha2 / (3.14159 * denom * denom);
        
        // Geometry
        float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
        float NdotV = saturate(dot(worldNormal, viewDir));
        float G = (NdotL / (NdotL * (1.0 - k) + k)) * (NdotV / (NdotV * (1.0 - k) + k));
        
        // Cook-Torrance BRDF
        float3 specular = (D * G * F) / (4.0 * NdotL * NdotV + 0.001);
        float3 diffuse = albedo / 3.14159;
        
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);
        
        finalColor += (kD * diffuse + specular) * SunColor * SunIntensity * NdotL * shadow;
    }
    
    // Global Illumination
    if (RTXIntensity > 0.0)
    {
        float3 gi = ComputeGlobalIllumination(input.WorldPosition, worldNormal, screenUV);
        finalColor += albedo * gi * ao;
        
        // RTX Reflections
        float3 reflections = ComputeReflections(input.WorldPosition, worldNormal, viewDir, roughness, screenUV);
        finalColor += reflections * metallic;
    }
    
    // Ambient lighting fallback
    float3 ambient = SampleEnvironmentMap(worldNormal) * AmbientIntensity * ao;
    finalColor += albedo * ambient;
    
    // Tone mapping
    finalColor = finalColor / (finalColor + 1.0); // Simple Reinhard
    finalColor = pow(finalColor, 1.0 / 2.2); // Gamma correction
    
    return float4(finalColor, diffuseSample.a);
}
