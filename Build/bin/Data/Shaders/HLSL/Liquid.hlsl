#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Transform.hlsl"

void VS(float4 iPos : POSITION,
    float2 iTexCoord : TEXCOORD0,
    float3 iNormal : NORMAL,
    out float2 oTexCoord : TEXCOORD0,
    out float4 oWorldPos : TEXCOORD2,
	out float3 oNormal : NORMAL,
    #ifdef VERTEXCOLOR
        out float4 oColor : COLOR0,
    #endif
    #if defined(D3D11) && defined(CLIPPLANE)
        out float oClip : SV_CLIPDISTANCE0,
    #endif
    out float4 oPos : OUTPOSITION)
{
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);
	float2 texCoord = worldPos.xz * 0.1;
    oTexCoord = texCoord;
    oWorldPos = float4(worldPos, GetDepth(oPos));
	oNormal = iNormal;

    #if defined(D3D11) && defined(CLIPPLANE)
        oClip = dot(oPos, cClipPlane);
    #endif
    
    #ifdef VERTEXCOLOR
        oColor = iColor;
    #endif
}

void PS(float2 iTexCoord : TEXCOORD0,
    float4 iWorldPos: TEXCOORD2,
	float3 iNormal: NORMAL,
    #ifdef VERTEXCOLOR
        float4 iColor : COLOR0,
    #endif
    #if defined(D3D11) && defined(CLIPPLANE)
        float iClip : SV_CLIPDISTANCE0,
    #endif
    out float4 oColor : OUTCOLOR0)
{
	float2 anim = float2(cElapsedTimePS, cElapsedTimePS);
	float2 nTex = iTexCoord - float2(cos(cElapsedTimePS * 0.5), sin(cElapsedTimePS * 0.5));
    float4 diffColor = cMatDiffColor * Sample2D(DiffMap, nTex);
	float4 noiseColor = Sample2D(EnvMap, (iTexCoord * 0.015625) + anim * 0.1);
	float4 diff2Color = cMatDiffColor * Sample2D(DiffMap, iTexCoord - float2(sin(cElapsedTimePS * 0.1), cos(cElapsedTimePS * 0.1)));
	diffColor -= diff2Color * 0.5;
	diffColor += noiseColor * 0.25;
	diffColor.a = 0.75;
    oColor = diffColor;
}
