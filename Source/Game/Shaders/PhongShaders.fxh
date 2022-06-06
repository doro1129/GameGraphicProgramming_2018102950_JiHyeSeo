//--------------------------------------------------------------------------------------
// File: PhongShaders.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#define NUM_LIGHTS (2)
#define NEAR_PLANE (0.01f)
#define FAR_PLANE (1000.0f)

Texture2D aTextures[2] : register(t0);
SamplerState aSamplers[2] : register(s0);

Texture2D shadowMapTexture : register(t2);
SamplerState shadowMapSampler : register(s2);

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement
  Summary:  Constant buffer used for view transformation and shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement : register(b0)
{
    matrix View;
    float4 CameraPosition;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize
  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame
  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutputColor;
    bool HasNormalMap;
};

struct PointLight
{
    float4 Position;
    float4 Color;
    float4 AttentionDistance;
};

cbuffer cbLights : register(b3)
{
    //float4 LightPositions[NUM_LIGHTS];
    //float4 LightColors[NUM_LIGHTS];
	//float LightAttenuationDistance[NUM_LIGHTS];
    PointLight PointLights[NUM_LIGHTS];
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_PHONG_INPUT
  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_PHONG_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    row_major matrix mTransform : INSTANCE_TRANSFORM;
};

struct PS_PHONG_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float4 LightViewPosition : TEXCOORD1;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_LIGHT_CUBE_INPUT
  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_LIGHT_CUBE_INPUT
{
    float4 Position : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_PHONG_INPUT VSPhong(VS_PHONG_INPUT input)
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT) 0;
    
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.Normal = normalize(mul(float4(input.Normal, 0), World).xyz);
    
    if (HasNormalMap)
    {
        output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }
    
    output.TexCoord = input.TexCoord;
    output.WorldPosition = mul(input.Position, World);
    
    output.LightViewPosition = mul(input.Position, World);
    //output.LightViewPosition = mul(output.LightViewPosition, LightViews[0]);
    //output.LightViewPosition = mul(output.LightViewPosition, LightProjections[0]);

    return output;
}

PS_LIGHT_CUBE_INPUT VSLightCube(VS_PHONG_INPUT input)
{
    PS_LIGHT_CUBE_INPUT output = (PS_LIGHT_CUBE_INPUT) 0;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    return output;
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return ((2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE))) / FAR_PLANE;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSPhong(PS_PHONG_INPUT input) : SV_Target
{   
    float3 normal = normalize(input.Normal);
    
    if (HasNormalMap)
    {
        //Sample the pixel in the normal map
        float4 bumpMap = aTextures[1].Sample(aSamplers[1], input.TexCoord);

        //Expand the range of the normal value from (0, +1) to (-1, +1)
        bumpMap = (bumpMap * 2.0) - 1.0f;
        
        //Calculate the normal from the data in the normal map
        float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);
        
        //Normalize the resulting bump normal and replace existing normal
        normal = normalize(bumpNormal);
    }
    
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    float3 store_ambient = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        float squaredR = dot(input.WorldPosition - PointLights[i].Position.xyz, input.WorldPosition - PointLights[i].Position.xyz);
        float lightAttenuation = saturate(PointLights[i].AttentionDistance.z / (squaredR + 0.000001f));
        store_ambient += ambient * aTextures[0].Sample(aSamplers[0], input.TexCoord).xyz * PointLights[i].Color.xyz * lightAttenuation;
    
        float3 lightDirection = normalize(input.WorldPosition - PointLights[i].Position.xyz);
        diffuse += max(dot(normalize(normal), -lightDirection), 0.0f) * PointLights[i].Color.xyz * aTextures[0].Sample(aSamplers[0], input.TexCoord).xyz * lightAttenuation;
    
        float3 reflectDirection = normalize(reflect(lightDirection, normal));
        specular += pow(max(dot(-viewDirection, reflectDirection), 0.0f), 20.0f) * PointLights[i].Color.xyz * aTextures[0].Sample(aSamplers[0], input.TexCoord).xyz * lightAttenuation;
    }

    return float4(saturate(specular + diffuse + store_ambient), 1.0f);
}

float4 PSLightCube(PS_LIGHT_CUBE_INPUT input) : SV_Target
{
    return OutputColor;
}