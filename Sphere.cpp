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
#include "Sphere.h"
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
Sphere::Sphere()
	: mNumVertices(0), mNumIndices(0),
	md3dDevice(0), mVB(0), mIB(0)
{
}

/***********************

* ~Terrain: Terrain Destructor
* @author: Thomas O'Brien

********************/
Sphere::~Sphere()
{
	ReleaseCOM(mVertexLayout);
	ReleaseCOM(mFX);
	ReleaseCOM(RSCullNone);
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mSRV);
}


/***********************

* init: Initialise the Sphere
* @author: Thomas O'Brien
* @parameter: ID3D10Device* - Pointer to directx10 device
* @parameter: float - Scale
* @return: void

********************/
void Sphere::init(ID3D10Device* device, int latLines /* = 10 */, int longLines /* = 10 */)
{
	//Set member variables
	md3dDevice = device;
	mRadius = 1.0f;
	mPosition = D3DXVECTOR3(10.0f, -8.0f, -8.0f);

	buildFX();
	buildVertexLayouts();
	
	mNumVertices = ((latLines - 2) * longLines) + 2;
	int iNumFaces = ((latLines - 3) * (longLines)* 2) + (longLines * 2);

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;

	mVertices.resize(mNumVertices);
	D3DXVECTOR3 currVertPos = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	D3DXVECTOR3 VertForward = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	mVertices[0].x = 0.0f;
	mVertices[0].y = 0.0f;
	mVertices[0].z = 1.0f;

	D3DXMATRIX rotationX, rotationY;

	for (int i = 0; i < latLines - 2; i++) //+= mFeatureSize)
	{
		spherePitch = (i + 1) * (PI / (latLines - 1));
		D3DXMatrixRotationX(&rotationX, spherePitch);
		for (int j = 0; j < longLines; j++)
		{
			sphereYaw = j * (2 * PI / (longLines));
			D3DXMatrixRotationZ(&rotationY, sphereYaw);
			D3DXMATRIX temp = rotationX * rotationY;
			D3DXVec3TransformNormal(&currVertPos, &VertForward, &temp);

			D3DXVec3Normalize(&currVertPos, &currVertPos);
			mVertices[i * longLines + j + 1] = currVertPos;
		}
	};

	mVertices[mNumVertices - 1].x = 0.0f;
	mVertices[mNumVertices - 1].y = 0.0f;
	mVertices[mNumVertices - 1].z = -1.0f;

	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(D3DXVECTOR3) * mNumVertices;
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &(mVertices.front());
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	std::vector<DWORD> indices(iNumFaces * 3);

	int k = 0;
	for (int l = 0; l < longLines - 1; ++l)
	{
		indices[k] = 0;
		indices[k + 1] = l + 1;
		indices[k + 2] = l + 2;
		k += 3;
	}

	indices[k] = 0;
	indices[k + 1] = longLines;
	indices[k + 2] = 1;
	k += 3;

	for (int i = 0; i < latLines - 3; ++i)
	{
		for (int j = 0; j < longLines - 1; ++j)
		{
			indices[k] = i*longLines + j + 1;
			indices[k + 1] = i*longLines + j + 2;
			indices[k + 2] = (i + 1)*longLines + j + 1;

			indices[k + 3] = (i + 1)*longLines + j + 1;
			indices[k + 4] = i*longLines + j + 2;
			indices[k + 5] = (i + 1)*longLines + j + 2;

			k += 6; // next quad
		}

		indices[k] = (i*longLines) + longLines;
		indices[k + 1] = (i*longLines) + 1;
		indices[k + 2] = ((i + 1)*longLines) + longLines;

		indices[k + 3] = ((i + 1)*longLines) + longLines;
		indices[k + 4] = (i*longLines) + 1;
		indices[k + 5] = ((i + 1)*longLines) + 1;

		k += 6;
	}

	for (int l = 0; l < longLines - 1; ++l)
	{
		indices[k] = mNumVertices - 1;
		indices[k + 1] = (mNumVertices - 1) - (l + 1);
		indices[k + 2] = (mNumVertices - 1) - (l + 2);
		k += 3;
	}

	indices[k] = mNumVertices - 1;
	indices[k + 1] = (mNumVertices - 1) - longLines;
	indices[k + 2] = mNumVertices - 2;

	D3D10_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * iNumFaces * 3;
	indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	mNumIndices = iNumFaces * 3;

	D3D10_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	md3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, &mIB);

}

/***********************

* draw: Draw terrain
* @author: Thomas O'Brien
* @parameter: D3DXMATRIX& - View Projection matrix
* @return: void

********************/
void Sphere::draw(D3DXMATRIX& rVP)
{
	mWVP = mWorld * rVP;
	predraw();

	UINT stride = sizeof(D3DXVECTOR3);
	UINT offset = 0;
	md3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dDevice->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
	md3dDevice->DrawIndexed(mNumIndices, 0, 0);

}

/***********************

* predraw: Call the predraw events
* @author: Thomas O'Brien
* @return: void

********************/
void Sphere::predraw()
{
	md3dDevice->IASetInputLayout(mVertexLayout);
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mfxWVPVar->SetMatrix((float*)&mWVP);

	md3dDevice->RSSetState(RSCullNone);

	D3D10_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc(&techDesc);

	mTechnique->GetPassByIndex(0)->Apply(0);
}

/***********************

* update: Update the terrain
* @author: Thomas O'Brien
* @parameter: D3DXVECTOR& - camera position
* @return: void

********************/
void Sphere::update()
{
	D3DXMatrixIdentity(&mWorld);
	D3DXVECTOR3 scale(mRadius, mRadius, mRadius);
	D3DXMATRIX matScale, matTranslation;

	D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);
	D3DXMatrixTranslation(&matTranslation, mPosition.x, mPosition.y, mPosition.z);

	mWorld = matScale * matTranslation;
}

void Sphere::transform(D3DXVECTOR3& _rTrans)
{
	mPosition += _rTrans;
}


void Sphere::buildFX()
{
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;
	hr = D3DX10CreateEffectFromFile(L"object.fx", 0, 0,
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

	mTechnique = mFX->GetTechniqueByName("ObjectTech");
	mfxWVPVar = mFX->GetVariableByName("gWVP")->AsMatrix();

	D3D10_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D10_RASTERIZER_DESC));
	cmdesc.CullMode = D3D10_CULL_NONE;
	cmdesc.FillMode = D3D10_FILL_SOLID;
	hr = md3dDevice->CreateRasterizerState(&cmdesc, &RSCullNone);
}

void Sphere::buildVertexLayouts()
{
	// Create the vertex input layout.

	D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	mTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 1, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &mVertexLayout));
}