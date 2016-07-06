#pragma once

//Local Includes
#include "Utility.h"
#include "Vertex.h"
#include "Particle.h"
#include "Terrain.h"

//Library Includes
#include <vector>



class DebugLines
{
	//Member Functions
public:
	DebugLines();
	~DebugLines();

	void init(ID3D10Device* pDevice);
//	void update(float dt);
	void draw(D3DXMATRIX* pWVP);
	void shutdown();
	void AddLine(CLine& _rLine);
	void clear();
private:
	void predraw();
	void buildFX();
	void buildVertexLayouts();
	void UpdateBuffer();

	//Member Variables
private:
	//Cloth dependancies
	std::vector<CLine> mLines;

	std::vector<Vertex> mVertices;

	int mMaxLines;
	int mNextIndex;

	//DX10 dependancies
	DWORD mNumVertices;

	ID3D10Device* md3dDevice;//Device
	ID3D10Buffer* mVB; // Vertex buffer
	ID3D10Buffer* mIB; // Index  buffer

	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTechnique;
	ID3D10InputLayout* mVertexLayout;
	ID3D10EffectMatrixVariable* mfxWVPVar;
};

