#pragma once
#include "Vertex.h"
#include "Utility.h"
#include "Sphere.h"
#include <vector>

class Capsule
{
public:
	Capsule();
	~Capsule();

	void buildRing(int segmentCount, D3DXVECTOR3 centre, float radius,
		float v, bool buildTriangles);

	void init(ID3D10Device* pDevice);
	void update();
	void predraw();
	void draw(D3DXMATRIX& rVP);
	void transform(D3DXVECTOR3& _rTrans);
	D3DXVECTOR3 getPos() { return mPosition; };
	float getRadius() { return mRadius; };
	float getHeight() { return mHeight; };

private:

	void buildFX();
	void buildVertexLayouts();

	//Member Variables

	std::vector<D3DXVECTOR3> mVertices;
	std::vector<DWORD> mIndices;
	
	DWORD mNumVertices;
	DWORD mNumIndices;

	ID3D10Device* md3dDevice;//Device
	ID3D10Buffer* mVB; // Vertex buffer
	ID3D10Buffer* mIB; // Index  buffer

	D3DXMATRIX mWorld;
	D3DXMATRIX mWVP;

	D3DXVECTOR3 mPosition;
	float mRadius;
	float mHeight;

	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTechnique;
	ID3D10InputLayout* mVertexLayout;
	ID3D10EffectMatrixVariable* mfxWVPVar;

	Sphere* mSpheres[2];
};

