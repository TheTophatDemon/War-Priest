#include "Uniforms.hlsl"
#include "Transform.hlsl"
#include "Samplers.hlsl"
#include "ScreenPos.hlsl"

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
	float lightness = (col.r+col.g+col.b) / 3.0;
	float factor = 16.0f - (10.0f * lightness);
	col *= factor;
	col.r = floor(col.r);
	col.g = floor(col.g);
	col.b = floor(col.b);
	col /= factor;
	oColor = col;
}