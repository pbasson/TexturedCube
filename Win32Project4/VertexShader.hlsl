

cbuffer ConstantBuffer
{
	matrix Projection;
	float3 ViewDir; // in world space
	//float accumulator;
}

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Nor : NORMAL;
	float2 UV0 : UV0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Nor : NORMAL;
	float2 UV0 : UV0;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;

	float4 pos = float4(input.Pos, 1);
	output.Pos = mul(pos, Projection);
	output.Nor = input.Nor;
	output.UV0 = input.UV0;
	return output;
} 