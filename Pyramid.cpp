#include "Pyramid.h"
#include "Utility.h"


Pyramid::Pyramid()
{
}


Pyramid::~Pyramid()
{
	ReleaseCOM(mVB); // Vertex buffer
	ReleaseCOM(mFX);
	ReleaseCOM(mVertexLayout);
}

void Pyramid::init(ID3D10Device* pDevice)
{
	md3dDevice = pDevice;
	
	mPosition = D3DXVECTOR3(10.0f, -8.0f, -8.0f);
	D3DXVECTOR3 mOrigin = D3DXVECTOR3(0, 0, 0);
	mScale = 4.0f;

	buildFX();
	buildVertexLayouts();

	D3DXVECTOR3 baseVerts[4];
	int iCount = 0;
	for (int x = -1; x < 2; x += 2)
	{
		for (int z = -1; z < 2; z += 2)
		{
			D3DXVECTOR3 base = mOrigin - D3DXVECTOR3(0, mScale, 0);
			baseVerts[iCount++] = base + D3DXVECTOR3( x * mScale * 0.5f, 0, z * mScale * 0.5f);
		}
	}
	
	mTriangles[0] = Triangle(mOrigin, baseVerts[0], baseVerts[1]);
	mTriangles[1] = Triangle(mOrigin, baseVerts[1], baseVerts[3]);
	mTriangles[2] = Triangle(mOrigin, baseVerts[3], baseVerts[2]);
	mTriangles[3] = Triangle(mOrigin, baseVerts[2], baseVerts[0]);
	mTriangles[4] = Triangle(baseVerts[1], baseVerts[0], baseVerts[2]);
	mTriangles[5] = Triangle(baseVerts[1], baseVerts[2], baseVerts[3]);

	calcNorm(&mNormals[0], mTriangles[0]);
	calcNorm(&mNormals[1], mTriangles[1]);
	calcNorm(&mNormals[2], mTriangles[2]);
	calcNorm(&mNormals[3], mTriangles[3]);


	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_IMMUTABLE;
	vbd.ByteWidth = 6 * sizeof(Triangle);
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = mTriangles;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	mNumVertices = 18;
}

void Pyramid::update()
{
	D3DXMatrixIdentity(&mWorld);
	//D3DXVECTOR3 scale(1.0f, , mRadius);
	D3DXMATRIX matScale, matTranslation;

	//D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);
	D3DXMatrixTranslation(&matTranslation, mPosition.x, mPosition.y, mPosition.z);

	mWorld = /*matScale **/ matTranslation;
}

void Pyramid::predraw()
{
	md3dDevice->IASetInputLayout(mVertexLayout);
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mfxWVPVar->SetMatrix((float*)&mWVP);

	//md3dDevice->RSSetState(RSCullNone);

	D3D10_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc(&techDesc);

	mTechnique->GetPassByIndex(0)->Apply(0);
}

void Pyramid::draw(D3DXMATRIX & rVP)
{
	mWVP = mWorld * rVP;

	predraw();

	UINT stride = sizeof(D3DXVECTOR3);
	UINT offset = 0;
	md3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	//md3dDevice->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
	md3dDevice->Draw(mNumVertices, 0);
}

void Pyramid::transform(D3DXVECTOR3 & _rTrans)
{
	mPosition += _rTrans;
}

void Pyramid::calcNorm(D3DXVECTOR3* output, Triangle & _tri)
{
	D3DXVECTOR3 x, y;

	D3DXVec3Subtract(&x, &_tri.b, &_tri.a);
	D3DXVec3Subtract(&y, &_tri.c, &_tri.a);
	D3DXVec3Cross(output, &x, &y);
	D3DXVec3Normalize(output, output);
	D3DXVec3Scale(output, output, -1.0f);
}

void Pyramid::buildFX()
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

	//D3D10_RASTERIZER_DESC cmdesc;
	//ZeroMemory(&cmdesc, sizeof(D3D10_RASTERIZER_DESC));
	//cmdesc.CullMode = D3D10_CULL_NONE;
	//cmdesc.FillMode = D3D10_FILL_SOLID;
	//hr = md3dDevice->CreateRasterizerState(&cmdesc, &RSCullNone);
}

void Pyramid::buildVertexLayouts()
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