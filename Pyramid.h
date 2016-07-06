#pragma once
#include "Vertex.h"
#include "Utility.h"
#include "Sphere.h"
#include <vector>

struct Triangle
{
	Triangle() {};
	Triangle(D3DXVECTOR3& _rA, D3DXVECTOR3& _rB, D3DXVECTOR3& _rC) : a(_rA), b(_rB), c(_rC) {};
	D3DXVECTOR3 a;
	D3DXVECTOR3 b;
	D3DXVECTOR3 c;

};

class Pyramid
{
public:
	Pyramid();
	~Pyramid();

	void init(ID3D10Device* pDevice);
	void update();
	void predraw();
	void draw(D3DXMATRIX& rVP);
	void transform(D3DXVECTOR3& _rTrans);
	D3DXVECTOR3 getPos() { return mPosition; };
	D3DXVECTOR3* getNormals() { return mNormals; };
	float getScale() { return mScale; };
private:
	void calcNorm(D3DXVECTOR3* output, Triangle& _tri);
	void buildFX();
	void buildVertexLayouts();

	//Member Variables

	std::vector<D3DXVECTOR3> mVertices;

	//ID3D10ShaderResourceView* mSRV;
	//ID3D10DepthStencilState* DSLessEqual;
	//ID3D10RasterizerState* RSCullNone;

	DWORD mNumVertices;

	ID3D10Device* md3dDevice;//Device
	ID3D10Buffer* mVB; // Vertex buffer

	D3DXMATRIX mWorld;
	D3DXMATRIX mWVP;

	D3DXVECTOR3 mPosition;
	float mScale;

	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTechnique;
	ID3D10InputLayout* mVertexLayout;
	ID3D10EffectMatrixVariable* mfxWVPVar;

	Triangle mTriangles[6];
	D3DXVECTOR3 mNormals[4];
	
};

