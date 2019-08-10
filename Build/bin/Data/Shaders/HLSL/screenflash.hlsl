#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Transform.hlsl"
#include "ScreenPos.hlsl"

uniform float4 cFlashColor;
uniform float cState;

void VS(float4 iPos : POSITION,
    out float2 oTexCoord : TEXCOORD0,
    out float2 oScreenPos : TEXCOORD1,
	out float2 oElapsedTime : TEXCOORD2,
    out float4 oPos : OUTPOSITION)
{
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);
    oTexCoord = GetQuadTexCoord(oPos);
    oScreenPos = GetScreenPosPreDiv(oPos);
	oElapsedTime.x = cElapsedTime;
	oElapsedTime.y = cElapsedTime;
}

void PS(float2 iTexCoord : TEXCOORD0,
    float2 iScreenPos : TEXCOORD1,
	float2 iElapsedTime : TEXCOORD2,
    out float4 oColor : OUTCOLOR0)
{
	float timeVal = sin((iElapsedTime.x + (iScreenPos.y * 5.0)) * 2.0);
	float2 tc = iTexCoord;
	if (cState > 0.0)
	{
		tc.x += timeVal * 0.01;
	}
	float4 col = Sample2D(DiffMap, tc);
	if (cState > 0.0)
	{
		if (cState == 1.0) col.r += ((iScreenPos.x + iScreenPos.y) * 0.5);
		if (cState == 2.0) col.b += ((iScreenPos.x + iScreenPos.y) * 0.5);
		col.rgb -= Sample2D(NormalMap, float2(timeVal * 0.01, iScreenPos.y)).rgb * 0.5;
	}
	col += (cFlashColor * cFlashColor.a);
	oColor = col;
}