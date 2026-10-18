#pragma once
#include <DirectXMath.h>
class Vertex
{
private:
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 normal;
public:
	Vertex() {};
	Vertex(float x, float y, float z, float u, float v,
		float normalX, float normalY, float normalZ)
		: pos(x, y, z), texCoord(u, v), normal(normalX, normalY, normalZ) {};

	//pos float4
	//norm float4
};

