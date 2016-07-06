#include "Capsule.h"
#include "Utility.h"


Capsule::Capsule()
{
}


Capsule::~Capsule()
{
	ReleaseCOM(mVB); // Vertex buffer
	ReleaseCOM(mIB); // Index  buffer
	ReleaseCOM(mFX);
	ReleaseCOM(mVertexLayout);

	for (int i = 0; i < 2; i++)
	{
		delete mSpheres[i];
		mSpheres[i] = nullptr;
	}

	md3dDevice = nullptr;
}

void Capsule::init(ID3D10Device* pDevice)
{
	md3dDevice = pDevice;
	int iRadialSegCount = 10;
	mPosition = D3DXVECTOR3(10.0f, -8.0f, -8.0f);
	mRadius = 1.0f;
	mHeight = 4.0f;

	buildFX();
	buildVertexLayouts();

	buildRing(iRadialSegCount, D3DXVECTOR3(0, 0, 0), mRadius, 0.0f, false);
	buildRing(iRadialSegCount, D3DXVECTOR3(0, 1.0f, 0) * mHeight, mRadius, 1.0f, true);
	
	mSpheres[0] = new Sphere();
	mSpheres[0]->init(md3dDevice, iRadialSegCount * 3, iRadialSegCount * 3);
	mSpheres[0]->transform(D3DXVECTOR3(0, mHeight, 0));
	mSpheres[1] = new Sphere();
	mSpheres[1]->init(md3dDevice, iRadialSegCount * 3, iRadialSegCount * 3);


	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(D3DXVECTOR3) * mVertices.size();
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &(mVertices.front());
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	mNumVertices = mVertices.size();

	D3D10_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * mIndices.size();
	indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	mNumIndices = mIndices.size();

	D3D10_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &mIndices.front();
	md3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, &mIB);
}

void Capsule::update()
{
	D3DXMatrixIdentity(&mWorld);
	//D3DXVECTOR3 scale(1.0f, , mRadius);
	D3DXMATRIX matScale, matTranslation;

	//D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);
	D3DXMatrixTranslation(&matTranslation, mPosition.x, mPosition.y, mPosition.z);

	mWorld = /*matScale **/ matTranslation;


	for (int i = 0; i < 2; i++)
	{
		mSpheres[i]->update();
	}

}

void Capsule::predraw()
{
	md3dDevice->IASetInputLayout(mVertexLayout);
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mfxWVPVar->SetMatrix((float*)&mWVP);

	//md3dDevice->RSSetState(RSCullNone);

	D3D10_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc(&techDesc);

	mTechnique->GetPassByIndex(0)->Apply(0);
}

void Capsule::draw(D3DXMATRIX & rVP)
{
	mWVP = mWorld * rVP;


	for (int i = 0; i < 2; i++)
	{
		mSpheres[i]->draw(rVP);
	}

	predraw();

	UINT stride = sizeof(D3DXVECTOR3);
	UINT offset = 0;
	md3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dDevice->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
	md3dDevice->DrawIndexed(mNumIndices, 0, 0);
}

void Capsule::transform(D3DXVECTOR3 & _rTrans)
{
	mPosition += _rTrans;

	for (int i = 0; i < 2; i++)
	{
		mSpheres[i]->transform(_rTrans);
	}
}

void Capsule::buildFX()
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

}

void Capsule::buildVertexLayouts()
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


void Capsule::buildRing(int segmentCount, D3DXVECTOR3 centre, float radius,
	float v, bool buildTriangles)
{
	float angleInc = (PI * 2.0f) / segmentCount;

	for (int i = 0; i <= segmentCount; i++)
	{
		float angle = angleInc * i;

		D3DXVECTOR3 unitPosition(0,0,0);
		unitPosition.x = cos(angle);
		unitPosition.z = sin(angle);

		mVertices.push_back(centre + unitPosition * radius);
		
		if (i > 0 && buildTriangles)
		{
			//May need to be (size-1) - 1
			int baseIndex = mVertices.size() - 1;

			int vertsPerRow = segmentCount + 1;

			int index0 = baseIndex;
			int index1 = baseIndex - 1;
			int index2 = baseIndex - vertsPerRow;
			int index3 = baseIndex - vertsPerRow - 1;

			mIndices.push_back(index0);
			mIndices.push_back(index2);
			mIndices.push_back(index1);

			mIndices.push_back(index2);
			mIndices.push_back(index3);
			mIndices.push_back(index1);
		}
	}
}

