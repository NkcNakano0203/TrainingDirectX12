#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

struct MeshVertex
{
	DirectX::XMFLOAT3 Postion;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
	DirectX::XMFLOAT3 Tangent;

	MeshVertex() = default;

	MeshVertex(
		DirectX::XMFLOAT3 const& postion,
		DirectX::XMFLOAT3 const& normal,
		DirectX::XMFLOAT2 const& texcoord,
		DirectX::XMFLOAT3 const& tangent)
		: Postion(postion)
		, Normal(normal)
		, TexCoord(texcoord)
		, Tangent(tangent)
	{}

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int InputElementCount = 4;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct Material
{
	DirectX::XMFLOAT3 Diffuse;	// 拡散反射成分
	DirectX::XMFLOAT3 Specular;	// 鏡面反射成分
	float Alpha;				// 透過成分
	float Shininess;			// 鏡面反射強度
	std::string DiffuseMap;		// テクスチャファイルパス
};

struct  Mesh
{
	std::vector<MeshVertex> Vertices;	// 頂点データ
	std::vector<uint32_t> Indices;		// 頂点インデックス
	uint32_t MaterialId;				// マテリアル番号
};

bool LoadMesh(
	const wchar_t* filename,
	std::vector<Mesh>& meshes,
	std::vector<Material>& materilas);