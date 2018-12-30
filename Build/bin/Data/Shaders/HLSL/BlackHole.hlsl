#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Transform.hlsl"
#include "Fog.hlsl"

void VS(float4 iPos : POSITION,
    out float2 oTexCoord : TEXCOORD0,
	out float4 oClipPos : TEXCOORD1,
    out float4 oWorldPos : TEXCOORD2,
    #if defined(D3D11) && defined(CLIPPLANE)
        out float oClip : SV_CLIPDISTANCE0,
    #endif
	#ifdef INSTANCED
        float4x3 iModelInstance : TEXCOORD4,
    #endif
    out float4 oPos : OUTPOSITION)
{
    float2 iTexCoord = float2(0.0, 0.0);
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);
	oClipPos = float4(mul(float4(worldPos, 1.0), cView), 1.0);
    oTexCoord = GetTexCoord(iTexCoord);
    oWorldPos = float4(worldPos, GetDepth(oPos));
    #if defined(D3D11) && defined(CLIPPLANE)
        oClip = dot(oPos, cClipPlane);
    #endif
}

void PS(float2 iTexCoord : TEXCOORD0,
	float4 iClipPos : TEXCOORD1,
    float4 iWorldPos: TEXCOORD2,
    #if defined(D3D11) && defined(CLIPPLANE)
        float iClip : SV_CLIPDISTANCE0,
    #endif
    out float4 oColor : OUTCOLOR0)
{
	float2 movement = float2(cElapsedTimePS, cElapsedTimePS);
    float4 diffColor = cMatDiffColor * Sample2D(DiffMap, (iClipPos.xy * 0.5 / iClipPos.z) + movement);
    oColor = diffColor;
}
