#pragma once

#include "IRenderable.h"
#include <vector>

using namespace DirectX;

struct UVVertex
{
	Vector3 Pos;
	Vector3 Normal;
	Vector2 UV;

};

enum{TOP, BOTTOM, FRONT, BACK, RIGHT, LEFT, NUM_FACE};

struct CubeFace
{
	Vector3 xDir;
	Vector3 yDir;
	Vector3 zDir; 
};

typedef unsigned int VertexIndex;

class Mesh: public IRenderable
{
	public:
		Mesh();
		~Mesh();

		void Initialise(DirectXWrapper *wrapper);
		void Render(DirectXWrapper *wrapper);
		

		void SetPosition(float x, float y, float z) { xpos = x; ypos = y; zpos = z; };
	
		static const CubeFace cubeFace[NUM_FACE];
	private:

		ID3D11Buffer* pVertices = nullptr;
		ID3D11Buffer* pIndices = nullptr;

		float angle = 0;

		float xpos=0, ypos=0, zpos= 0;

		ID3D11Resource *texture;
		ID3D11ShaderResourceView *textureView;

		ID3D11VertexShader*		 pVertexShader = nullptr;
		ID3D11InputLayout*		 pVertexLayout = nullptr;
		ID3D11PixelShader*		 pPixelShader = nullptr;
		ID3D11SamplerState*	     pSamplerLinear = nullptr; 

		int numVertices = 0;
		int numIndices = 0;

		Vector3 GetFaceNormal(UVVertex *verts, VertexIndex i0, VertexIndex i1, VertexIndex i2);

		void CreateMesh(UVVertex **pUVVertices, VertexIndex **pIndices);

};