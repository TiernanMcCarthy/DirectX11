//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

cbuffer cbChangeOnResize : register( b1 )
{
    matrix Projection;
};

cbuffer cbChangesEveryFrame : register( b2 )
{
    matrix World;
	matrix View2;
	matrix Projection2;
    float4 vMeshColor;

	float4 vLightDir[2]; //Addition of lighting variables from tutorial 06
	float4 vLightColor[2];
	float4 vOutputColor;

};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
	float3 Norm: NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
	float3 Norm: TEXCOORD1;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View2 );
    output.Pos = mul( output.Pos, Projection );
    output.Tex = input.Tex;
	output.Norm = mul(float4(input.Norm, 1), World).xyz;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 finalColor = 0;
	float4 vLightColorlocal = float4(0, 0, 1, 0);
	//do NdotL lighting for 2 lights
	for (int i = 0; i < 2; i++)
	{
		finalColor += saturate(dot((float3)vLightDir[i],input.Norm) * vLightColor[i]);
	}
	finalColor += vMeshColor;
	finalColor.a = 1;
	//return finalColor;
	//return finalColor;
    return txDiffuse.Sample( samLinear, input.Tex ) +(finalColor);
}
//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}
