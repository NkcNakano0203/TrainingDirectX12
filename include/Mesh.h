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
	DirectX::XMFLOAT3 Diffuse;	// �g�U���ː���
	DirectX::XMFLOAT3 Specular;	// ���ʔ��ː���
	float Alpha;				// ���ߐ���
	float Shininess;			// ���ʔ��ˋ��x
	std::string DiffuseMap;		// �e�N�X�`���t�@�C���p�X
};

struct  Mesh
{
	std::vector<MeshVertex> Vertices;	// ���_�f�[�^
	std::vector<uint32_t> Indices;		// ���_�C���f�b�N�X
	uint32_t MaterialId;				// �}�e���A���ԍ�
};

bool LoadMesh(
	const wchar_t* filename,
	std::vector<Mesh>& meshes,
	std::vector<Material>& materilas);