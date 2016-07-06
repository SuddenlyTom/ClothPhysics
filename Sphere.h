//=======================================================================================
// Cube.h                                                                         
//=======================================================================================

#ifndef Sphere_H
#define Sphere_H


#include "Utility.h"

#include "Vertex.h"
#include <vector>

class Sphere
{
public:
	Sphere();
	~Sphere();

	void init(ID3D10Device* device, int latLines = 10, int longLines = 10);
	void update();
	void predraw();
	void draw(D3DXMATRIX& rVP);
	void transform(D3DXVECTOR3& _rTrans);
	D3DXVECTOR3 getPos() { return mPosition; };
	float getRadius() { return mRadius; };

private:

	void buildFX();
	void buildVertexLayouts();

	//Member Variables

	ID3D10ShaderResourceView* mSRV;
	ID3D10DepthStencilState* DSLessEqual;
	ID3D10RasterizerState* RSCullNone;

	DWORD mNumVertices;
	DWORD mNumIndices;

	ID3D10Device* md3dDevice;//Device
	ID3D10Buffer* mVB; // Vertex buffer
	ID3D10Buffer* mIB; // Index  buffer

	D3DXMATRIX mWorld;
	D3DXMATRIX mWVP;

	D3DXVECTOR3 mPosition;
	float mRadius;

	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTechnique;
	ID3D10InputLayout* mVertexLayout;
	ID3D10EffectMatrixVariable* mfxWVPVar;
	ID3D10EffectShaderResourceVariable* mfxSRV;

	std::vector<D3DXVECTOR3> mVertices;
};

#endif // Sphere_H