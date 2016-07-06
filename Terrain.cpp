//
//	Bachelor of Software Engineering (Game Programming)
//	Media Design School
//	Auckland
//	New Zealand
//
//	(c) 2015 Media Design School
//
//	File Name		:		Terrain.cpp
//	Description		:		Implementation of the Terrain class
//	Author			:		Tom O'Brien
//	Email			:		Thomas.OBrien@mediadesign.school.nz
//

//Local Includes
#include "Terrain.h"
#include "Vertex.h"
//Library Includes
#include <vector>
#include <iostream>
#include <string>
#include <fstream>


/***********************

* Terrain: Terrain Constructor
* @author: Thomas O'Brien

********************/
Terrain::Terrain()
: mNumVertices(0), mNumIndices(0),
  md3dDevice(0), mVB(0), mIB(0)
{
}

/***********************

* ~Terrain: Terrain Destructor
* @author: Thomas O'Brien

********************/
Terrain::~Terrain()
{
	ReleaseCOM(mVertexLayout);
	ReleaseCOM(mFX);

	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
		
	ReleaseCOM(mrvTerrain);
}


/***********************

* init: Initialise the Terrain
* @author: Thomas O'Brien
* @parameter: ID3D10Device* - Pointer to directx10 device
* @parameter: float - Scale
* @return: void

********************/
void Terrain::init(ID3D10Device* device, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 scale, int gridSize)
{
	//Set member variables
	md3dDevice = device;
	mGridSize = gridSize;

	buildFX();
	buildVertexLayouts();

	mvecPosition = pos;
	mvecRotation = rot;
	mvecScale = scale;
	
		//Calculate vertices and indices
	mNumVertices = gridSize * gridSize;
	mNumIndices = (gridSize * 2) * (gridSize - 1) + (gridSize - 2);
	
	float UVscale = scale.x;
	float fGrid = static_cast<float>(gridSize);

	mVertices.reserve(mNumVertices);
	for (int y = 0; y < gridSize; y++) //+= mFeatureSize)
	{
		for (int x = 0; x < gridSize; x++)// += mFeatureSize)
		{
			TexturedVertex newVert = TexturedVertex(D3DXVECTOR3(static_cast<float>(x), 0, static_cast<float>(y)), D3DXVECTOR2(static_cast<float>(x) /** (UVscale / fGrid)*/, static_cast<float>(y) /** (UVscale / fGrid)*/));
			mVertices.push_back(newVert);;
		}
	};

	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(TexturedVertex) * mNumVertices;
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &(mVertices.front());
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	DWORD* ppIndices = new DWORD[mNumIndices];

	DWORD index = 0;
	for (int z = 0; z < gridSize - 1; z++)
	{
		if (z % 2 == 0)
		{
			int x;
			for (x = 0; x < gridSize; x++)
			{
				(ppIndices)[index++] = x + (z * gridSize);
				(ppIndices)[index++] = x + (z * gridSize) + gridSize;
			}

			if (z != gridSize - 2)
			{
				(ppIndices)[index++] = --x + (z * gridSize);
			}
		}
		else
		{
			int x;
			for (x = gridSize - 1; x >= 0; x--)
			{
				(ppIndices)[index++] = x + (z * gridSize);
				(ppIndices)[index++] = x + (z * gridSize) + gridSize;
			}

			if (z != gridSize - 2)
			{
				(ppIndices)[index++] = ++x + (z * gridSize);
			}
		}
	}

	D3D10_BUFFER_DESC ibd;
	ibd.Usage = D3D10_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD) * mNumIndices;
	ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = ppIndices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));

	delete[] ppIndices;
	ppIndices = nullptr;
}

/***********************

* draw: Draw terrain
* @author: Thomas O'Brien
* @parameter: D3DXMATRIX& - View Projection matrix
* @return: void

********************/
void Terrain::draw()
{
	this->predraw();

	mfxWVPVar->SetMatrix((float*)&mWVP);
	mfxTextureVar->SetResource(mrvTerrain);

	D3D10_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTechnique->GetPassByIndex(p)->Apply(0);

		UINT stride = sizeof(TexturedVertex);
		UINT offset = 0;
		md3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
		md3dDevice->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
		md3dDevice->DrawIndexed(mNumIndices, 0, 0);
	}
}

/***********************

* predraw: Call the predraw events
* @author: Thomas O'Brien
* @return: void

********************/
void Terrain::predraw()
{
	md3dDevice->IASetInputLayout(mVertexLayout);
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

/***********************

* buildVertexLayouts: Build the vertex layouts
* @author: Thomas O'Brien
* @return: void

********************/
void Terrain::buildVertexLayouts()
{
	// Create the vertex input layout.

	D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	mTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &mVertexLayout));

}


/***********************

* buildFX: Build the shader fx
* @author: Thomas O'Brien
* @return: void

********************/
void Terrain::buildFX()
{
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;
	hr = D3DX10CreateEffectFromFile(L"ground.fx", 0, 0,
		"fx_4_0", shaderFlags, 0, md3dDevice, 0, 0, &mFX, &compilationErrors, 0);
	if (FAILED(hr))
	{
		if (compilationErrors)
		{
			MessageBoxA(0, (char*)compilationErrors->GetBufferPointer(), 0, 0);
			ReleaseCOM(compilationErrors);
		}
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX10CreateEffectFromFile", true);
	}

	mTechnique = mFX->GetTechniqueByName("GroundTech");
	mfxWVPVar = mFX->GetVariableByName("gWVP")->AsMatrix();
	mfxTextureVar = mFX->GetVariableByName("gTexture")->AsShaderResource();

	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"Assets/texture.jpg", 0, 0, &mrvTerrain, 0);
	

}

void Terrain::update(D3DXMATRIX& rVP)
{
	D3DXMATRIX matRotX, matRotY, matRotZ, matTrans, matScale;

												// Calculate rotation matrix
	D3DXMatrixRotationX(&matRotX, mvecRotation.x);        // Pitch
	D3DXMatrixRotationY(&matRotY, mvecRotation.y);        // Yaw
	D3DXMatrixRotationZ(&matRotZ, mvecRotation.z);        // Roll

												// Calculate a translation matrix
	D3DXMatrixTranslation(&matTrans, mvecPosition.x, mvecPosition.y, mvecPosition.z);

												// Calculate a scale matrix
	D3DXMatrixScaling(&matScale, mvecScale.x, 1.0f, mvecScale.z);

	// Calculate our world matrix by multiplying the above (in the correct order)
	mWorld = matScale * (matRotX*matRotY*matRotZ) * matTrans;

	mWVP = mWorld * rVP;
}
