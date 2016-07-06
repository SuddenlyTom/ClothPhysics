#include "Utility.h"

#ifndef VERTEX_H
#define VERTEX_H

struct Vertex
{
	Vertex(){};
	Vertex(D3DXVECTOR3 newPos, D3DXCOLOR newcolor):pos(newPos),color(newcolor){	};
	D3DXVECTOR3 pos;
	D3DXCOLOR   color;
};

struct VertexParticle
{
	VertexParticle(){};
	VertexParticle(D3DXVECTOR3 newPos, D3DXVECTOR2 newScale):pos(newPos),scale(newScale){	};
	D3DXVECTOR3 pos;
	D3DXVECTOR2 scale;
};

struct VertexWithNormal
{
	VertexWithNormal() {};
	VertexWithNormal(D3DXVECTOR3 newPos, D3DXVECTOR3 newNormal) :pos(newPos), normal(newNormal) {	};

	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
};

struct TexturedVertex
{
	TexturedVertex() {};
	TexturedVertex(D3DXVECTOR3 inPos, D3DXVECTOR2 inUV) :pos(inPos), uv(inUV) {};
	D3DXVECTOR3 pos;
	D3DXVECTOR2 uv;
};

struct TexturedVertexWithNormal
{
	TexturedVertexWithNormal(){};
	TexturedVertexWithNormal(D3DXVECTOR3 inPos, D3DXVECTOR3 inNormal, D3DXVECTOR2 inUV):pos(inPos),normal(inNormal),uv(inUV){};
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 uv;
};

#endif // VERTEX_H