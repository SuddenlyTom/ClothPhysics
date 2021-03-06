
cbuffer cbPerFrame
{
	float4x4 gWVP; 
};

Texture2D gTexture;

SamplerState gTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
};

struct VS_IN
{
	float3 posL    : POSITION;
	float2 uv	: UV;
};

struct VS_OUT
{
	float4 posH    : SV_POSITION;
    float3 posL    : POSITION;
    float2 uv : UV;
};
 
VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;

	vOut.posL = vIn.posL;

	vOut.posH = mul(float4(vIn.posL, 1.0f), gWVP);

	vOut.uv = vIn.uv;

	return vOut;
}

float4 PS(VS_OUT pIn) : SV_Target
{
    return gTexture.Sample(gTriLinearSam, pIn.uv);
}

technique10 GroundTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}
