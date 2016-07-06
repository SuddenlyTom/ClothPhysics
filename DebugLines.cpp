#include "DebugLines.h"
#include "Vertex.h"
#include <algorithm>

DebugLines::DebugLines()
{
}


DebugLines::~DebugLines()
{
	shutdown();
}

void DebugLines::init(ID3D10Device* pDevice)
{
	md3dDevice = pDevice;
	mNextIndex = 0;
	mMaxLines = 6;

	mLines.resize(mMaxLines);

	//Initialise DirectX elements
	buildFX();

	mNumVertices = mMaxLines * 2;

	//Generate Vertex structures
	mVertices.resize(mNumVertices);
	
	//Generate Vertex buffer
	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * mNumVertices;
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &(mVertices.front());
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

}

void DebugLines::predraw()
{
	md3dDevice->IASetInputLayout(mVertexLayout);
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	//md3dDevice->RSSetState(mRS);
}


void DebugLines::draw(D3DXMATRIX* pWVP)
{
	UpdateBuffer();

	predraw();

	mfxWVPVar->SetMatrix((float*)pWVP);

	D3D10_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTechnique->GetPassByIndex(p)->Apply(0);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		md3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
		//md3dDevice->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
		//md3dDevice->DrawIndexed(mNumIndices, 0, 0);
		md3dDevice->Draw(mNumVertices, 0);
	}
}

void DebugLines::buildFX()
{
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;
	hr = D3DX10CreateEffectFromFile(L"cloth.fx", 0, 0,
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

	mTechnique = mFX->GetTechniqueByName("ClothTech");
	mfxWVPVar = mFX->GetVariableByName("gWVP")->AsMatrix();

	//mfxLightVar = mFX->GetVariableByName("gLight");
	//mfxEyePosVar = mFX->GetVariableByName("gEyePosW");


	buildVertexLayouts();
}

void DebugLines::buildVertexLayouts()
{
	// Create the vertex input layout.

	D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	mTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &mVertexLayout));
}

void DebugLines::UpdateBuffer()
{
	D3DXCOLOR lineColor = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);

	ZeroMemory(&mVertices[0], sizeof(Vertex) * mVertices.size());

	int iIndex = 0;
	for (unsigned int i = 0; i < mLines.size(); i++)
	{
		mVertices[iIndex].pos = mLines[i].pos1;
		mVertices[iIndex++].color = lineColor;

		mVertices[iIndex].pos = mLines[i].pos2;
		mVertices[iIndex++].color = lineColor;
	}

	void* pVerts;
	mVB->Map(D3D10_MAP_WRITE_DISCARD, 0, &pVerts);
	//ZeroMemory(pVerts, sizeof(Vertex) * mNumVertices);
	memcpy_s(pVerts, sizeof(Vertex) * mNumVertices, &mVertices[0], sizeof(Vertex) * mNumVertices);
	mVB->Unmap();
}

void DebugLines::shutdown()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mVertexLayout);
	md3dDevice = nullptr;

}

void DebugLines::AddLine(CLine& _rLine)
{
	mLines[mNextIndex++] = _rLine;
	
	if(mNextIndex == mMaxLines)
		mNextIndex = 0;
}

void DebugLines::clear()
{
	ZeroMemory(&mLines[0], sizeof(CLine) * mLines.size());
	ZeroMemory(&mVertices[0], sizeof(Vertex) * mVertices.size());
	mNextIndex = 0;
	UpdateBuffer();
}