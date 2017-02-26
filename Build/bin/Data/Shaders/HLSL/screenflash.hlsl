#include "Uniforms.hlsl"
#include "Transform.hlsl"
#include "Samplers.hlsl"
#include "ScreenPos.hlsl"

uniform float4 cFlashColor;

void VS(float4 iPos : POSITION,
    out float2 oTexCoord : TEXCOORD0,
    out float2 oScreenPos : TEXCOORD1,
    out float4 oPos : OUTPOSITION)
{
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);
    oTexCoord = GetQuadTexCoord(oPos);
    oScreenPos = GetScreenPosPreDiv(oPos);
}

void PS(float2 iTexCoord : TEXCOORD0,
    float2 iScreenPos : TEXCOORD1,
    out float4 oColor : OUTCOLOR0)
{
	float4 col = Sample2D(DiffMap, iTexCoord);
	col += (cFlashColor * cFlashColor.a);
	oColor = col;
}