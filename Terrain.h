//=======================================================================================
// Cube.h                                                                         
//=======================================================================================

#ifndef Cube_H
#define Cube_H


#include "Utility.h"

#include "Vertex.h"
#include <vector>

class Terrain
{
public:
	Terrain();
	~Terrain();
	 
	void init(ID3D10Device* device, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 scale, int gridSize);
	void update(D3DXMATRIX& rVP);
	void predraw();
	void draw();
	D3DXVECTOR3& GetPosition() { return mvecPosition; }; 
	D3DXVECTOR3& GetScale() { return mvecScale; };
	D3DXVECTOR3& GetRotation() { return mvecRotation; };

private:
	
	void buildFX();
	void buildVertexLayouts();

	DWORD mNumVertices;
	DWORD mNumIndices;

	ID3D10Device* md3dDevice;//Device
	ID3D10Buffer* mVB; // Vertex buffer
	ID3D10Buffer* mIB; // Index  buffer

	D3DXMATRIX mWorld;
	D3DXMATRIX mWVP;

	D3DXVECTOR3 mvecPosition;
	D3DXVECTOR3 mvecRotation;
	D3DXVECTOR3 mvecScale;

	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTechnique;
	ID3D10InputLayout* mVertexLayout;
	ID3D10EffectMatrixVariable* mfxWVPVar;
	ID3D10EffectShaderResourceVariable* mfxTextureVar;
	ID3D10ShaderResourceView* mrvTerrain;

	int mGridSize;
	std::vector<TexturedVertex> mVertices;
};

#endif // Cube_H