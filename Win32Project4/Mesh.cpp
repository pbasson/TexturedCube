#include "stdafx.h"
#include "Mesh.h"
#include "DirectXWrapper.h"
#include "WICTextureLoader.h"

#include "CompiledPixelShader.h"
#include "CompiledVertexShader.h"


Mesh::Mesh()
{

};

Mesh::~Mesh() //Decontructor of pointers
{
	if (pSamplerLinear) pSamplerLinear->Release();
	if (pVertices) pVertices->Release();
	if (pIndices) pIndices->Release();
	if (pPixelShader) pPixelShader->Release();
	if (pVertexLayout) pVertexLayout->Release();
	if (pVertexShader) pVertexShader->Release();
	if (texture) texture->Release();
	if (textureView) textureView->Release();

};

Vector3 Mesh::GetFaceNormal(UVVertex *verts, VertexIndex i0, VertexIndex i1, VertexIndex i2)
{
	Vector3 v0 = verts[i1].Pos - verts[i0].Pos;
	Vector3 v1 = verts[i2].Pos - verts[i1].Pos;
	
	Vector3 nor = v0.Cross(v1);
	nor.Normalize();
	return nor; 

};

const CubeFace Mesh::cubeFace[NUM_FACE]
{
	{ { 1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 } }, // TOP
	{ { 1, 0, 0 }, { 0, 0, -1 }, { 0, -1, 0 } }, // BOTTOM
	{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, -1 } }, // FRONT
	{ { -1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } }, // BACK
	{ { 0, 0, 1 }, { 0, 1, 0 }, { 1, 0, 0 } }, // RIGHT
	{ { 0, 0, -1 }, { 0, 1, 0 }, { -1, 0, 0 } } // LEFT
};

//CREATE THE MESH
void Mesh::CreateMesh(UVVertex **pUVVertices, VertexIndex **pIndices)
{
	assert(pUVVertices);
	assert(pIndices);
	
	*pUVVertices = new UVVertex[NUM_FACE*4];
	numVertices = NUM_FACE * 4; 

	for (int i = 0; i < NUM_FACE; i++)
	{
		(*pUVVertices)[i * 4 + 0].Pos = (cubeFace[i].xDir + cubeFace[i].yDir + cubeFace[i].zDir);
		(*pUVVertices)[i * 4 + 1].Pos = (cubeFace[i].xDir - cubeFace[i].yDir + cubeFace[i].zDir);
		(*pUVVertices)[i * 4 + 2].Pos = (-cubeFace[i].xDir - cubeFace[i].yDir + cubeFace[i].zDir);
		(*pUVVertices)[i * 4 + 3].Pos = (-cubeFace[i].xDir + cubeFace[i].yDir + cubeFace[i].zDir);
		(*pUVVertices)[i * 4 + 0].Normal = (cubeFace[i].zDir);
		(*pUVVertices)[i * 4 + 1].Normal = (cubeFace[i].zDir);
		(*pUVVertices)[i * 4 + 2].Normal = (cubeFace[i].zDir);
		(*pUVVertices)[i * 4 + 3].Normal = (cubeFace[i].zDir);
		(*pUVVertices)[i * 4 + 0].UV = Vector2(0, 0);
		(*pUVVertices)[i * 4 + 1].UV = Vector2(1, 0);
		(*pUVVertices)[i * 4 + 2].UV = Vector2(1, 1);
		(*pUVVertices)[i * 4 + 3].UV = Vector2(0, 1);
	}

	*pIndices = new VertexIndex[NUM_FACE * 6];
	numIndices = NUM_FACE * 6;


	for (int i = 0; i < NUM_FACE; i++)
	{
		(*pIndices)[i * 6 + 0] = i * 4 + 0;
		(*pIndices)[i * 6 + 1] = i * 4 + 2;
		(*pIndices)[i * 6 + 2] = i * 4 + 3;
		(*pIndices)[i * 6 + 3] = i * 4 + 0;
		(*pIndices)[i * 6 + 4] = i * 4 + 1;
		(*pIndices)[i * 6 + 5] = i * 4 + 2;
	}


};


void Mesh::Initialise(DirectXWrapper *wrapper)
{
	UVVertex *pUVVertices = nullptr;
	VertexIndex *pUVIndices = nullptr;

	CreateMesh(&pUVVertices, &pUVIndices);
	assert(pUVVertices);
	assert(pUVIndices);


	//ADD VERTICES TO D-BUFFER
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = numVertices*sizeof(UVVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = pUVVertices;
	HRESULT hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &pVertices);
	assert(SUCCEEDED(hr));
	delete []pUVVertices;
	
	//ADD INDICES TO D-BUFFER
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = numIndices*sizeof(VertexIndex);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = pUVIndices;
	hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &pIndices);
	assert(SUCCEEDED(hr));
	delete[]pUVIndices;

	//CREATE VERTEX SHADER
	hr = wrapper->GetDevice()->CreateVertexShader(g_CompiledVertexShader, sizeof(g_CompiledVertexShader), nullptr, &pVertexShader);
	assert(SUCCEEDED(hr));
	//CREATE PIXEL SHADER
	hr = wrapper->GetDevice()->CreatePixelShader(g_CompiledPixelShader, sizeof(g_CompiledPixelShader), nullptr, &pPixelShader);
	assert(SUCCEEDED(hr));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },	
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },		
		{ "UV",0,DXGI_FORMAT_R32G32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 }, 
	};

	UINT numElements = ARRAYSIZE(layout);
	wrapper->GetDevice()->CreateInputLayout(layout, numElements, g_CompiledVertexShader, sizeof(g_CompiledVertexShader), &pVertexLayout);
	
	//LOAD THE TEXTURE
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"tex1.jpg", &texture, &textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed Texture");
	}

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; 
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX; 

	hr = wrapper->GetDevice()->CreateSamplerState(&sampDesc, &pSamplerLinear);
	

}


void Mesh::Render(DirectXWrapper *wrapper)
{	//WORLD MATRIX
	XMMATRIX worldTransform = XMMatrixTranslation(xpos, ypos, zpos);
	wrapper->SetWorldTransform(worldTransform);

	wrapper->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	UINT strike = sizeof(UVVertex);
	UINT offset = 0;

	wrapper->GetContext()->IASetVertexBuffers(0, 1, &pVertices, &strike, &offset);
	
	wrapper->GetContext()->IASetIndexBuffer(pIndices, DXGI_FORMAT_R32_UINT, 0);

	wrapper->GetContext()->IASetInputLayout(pVertexLayout);

	wrapper->GetContext()->VSSetShader(pVertexShader, nullptr, 0);
	wrapper->GetContext()->PSSetShader(pPixelShader, nullptr, 0);

	wrapper->GetContext()->DrawIndexed(numIndices, 0, 0);
	wrapper->GetContext()->PSSetShaderResources(0, 1, &textureView);
	wrapper->GetContext()->PSSetSamplers(0,1, &pSamplerLinear);


}

