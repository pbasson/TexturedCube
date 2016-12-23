// VS will compile this to a header file called CompiledPixelShader.h
// It's a really basic pixel shader, it will be called every time your graphics hardware needs to know the colour of a pixel.
// It samples the texture and returns the colour it gets


Texture2D txDiffuse;
SamplerState samLinear;

cbuffer ConstantBuffer
{
	matrix Projection;
	float3 ViewDir;
	//float accumulator;
};

struct PS_INPUT
{
	float4 Pos: SV_POSITION;
	float3 Nor : NORMAL;
	float2 UV0 : UV0;
};

float4 main(PS_INPUT input) : SV_Target
{

	float3 ambient = { 0.1, 0.1, 0.1 };

	// we're going to hard code the single light direction and light colour to make the example simpler - we would pass these in as shader constants normally
	float3 lightDir = { 0.707, 0.707, 0 }; // actually the inverse of the light direction as we need the inverse for the dot products
	float3 lightCol = { 1, 1, 1 }; // the colour and brightness of our light source

	float shininess = 0.2;
//	float3 texColour = txDiffuse.Sample(samLinear, input.Tex);

	// we've been passed an interpolated normal, unfortunately normals don't survive interpolation so well, we need to renormalise it to make it unit length
	float3 normal = normalize(input.Nor);

		// use hard coded colour
		//float3 texColour = { 0.169, 0.396, 0.925 }; // ocean blue

		// diffuse = lightCol * (light direction . normal)
	float3 diffuseLight = lightCol * saturate(dot(normal, lightDir));
	float4 texCol = txDiffuse.Sample(samLinear, input.UV0);
	float4 tex = dot(diffuseLight, texCol);
	float3 tex1 = lightCol*texCol;
		// specular - (H.N)^n, we'll use 20 for n

	//float3 halfVector = normalize((ViewDir + lightDir) / 2);
//	float specIntensity = saturate(dot(normal, halfVector));
//	float3 specularLight = lightCol * pow(specIntensity, 500) * shininess;
	float3 lights = saturate(ambient*texCol + diffuseLight*texCol);
		// put it all together remembering that ambient and diffuse affect texture colour, specular ignores the texture		float3 lights = saturate(ambient*texColour + diffuseLight*texColour + specularLight);
	
	
	return float4(texCol);


}