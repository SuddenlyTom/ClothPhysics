
cbuffer cbPerFrame
{
	float4x4 gWVP;
};

SamplerState gTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
};

struct VS_IN
{
	float3 posL	   : POSITION;
	float4 color   : COLOR;
};

struct VS_OUT
{
	float4 posH    : SV_POSITION;
	float3 posL    : POSITION;
	float4 color   : COLOR;
};

VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;

	vOut.posL = vIn.posL;

	vOut.posH = mul(float4(vIn.posL, 1.0f), gWVP);
	
	vOut.color = vIn.color;

	return vOut;
}

float4 PS(VS_OUT pIn) : SV_Target
{
	return pIn.color;
}

technique10 ClothTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}
