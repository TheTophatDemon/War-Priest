#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Transform.hlsl"
#include "Fog.hlsl"

void VS(float4 iPos : POSITION,
    float2 iTexCoord : TEXCOORD0,
    out float2 oTexCoord : TEXCOORD0,
    out float4 oWorldPos : TEXCOORD2,
	#ifdef INSTANCED
        float4x3 iModelInstance : TEXCOORD4,
    #endif
    #if defined(D3D11) && defined(CLIPPLANE)
        out float oClip : SV_CLIPDISTANCE0,
    #endif
	out float4 oClipPos : TEXCOORD3,
    out float4 oPos : OUTPOSITION)
{
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);
	oClipPos = oPos;
	float2 texCoord = worldPos.xz * 0.1;
    oTexCoord = texCoord;
    oWorldPos = float4(worldPos, GetDepth(oPos));
    #if defined(D3D11) && defined(CLIPPLANE)
        oClip = dot(oPos, cClipPlane);
    #endif
}

void PS(float2 iTexCoord : TEXCOORD0,
    float4 iWorldPos: TEXCOORD2,
    #if defined(D3D11) && defined(CLIPPLANE)
        float iClip : SV_CLIPDISTANCE0,
    #endif
	float4 iClipPos : TEXCOORD3,
    out float4 oColor : OUTCOLOR0)
{
	float2 anim = float2(cElapsedTimePS, cElapsedTimePS);
	float2 nTex = iTexCoord - float2(cos(cElapsedTimePS * 0.5), sin(cElapsedTimePS * 0.5));
    float4 diffColor = cMatDiffColor * Sample2D(DiffMap, nTex);
	float4 noiseColor = Sample2D(EnvMap, (iTexCoord * 0.015625) + anim * 0.1);
	float4 diff2Color = cMatDiffColor * Sample2D(DiffMap, iTexCoord - float2(sin(cElapsedTimePS * 0.1), cos(cElapsedTimePS * 0.1)));
	diffColor -= diff2Color * 0.5;
	diffColor += noiseColor * 0.25;
	
	//float2 sourceSample = (iClipPos.xy * float2(1.0, -1.0) / iClipPos.w + float2(1.0, 1.0)) / 2.0;
	//float4 sourceColor = Sample2D(NormalMap, sourceSample);
	
    oColor = diffColor;
}
