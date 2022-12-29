#pragma once

// �C���N���[�h
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

// Tranform�\����
struct alignas(256) Transform
{
	DirectX::XMMATRIX   World;	// ���[���h�s��
	DirectX::XMMATRIX   View;	// �r���[�s��
	DirectX::XMMATRIX   Proj;	// �ˉe�s��
};

template<typename T> struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;		// �萔�o�b�t�@�̍\���ݒ�
	D3D12_CPU_DESCRIPTOR_HANDLE     HandleCPU;	// CPU�f�B�X�N���v�^�n���h��
	D3D12_GPU_DESCRIPTOR_HANDLE     HandleGPU;	// GPU�f�B�X�N���v�^�n���h��
	T* pBuffer;									// �o�b�t�@�擪�ւ̃|�C���^
};

class App
{
public:
	// �p�u���b�N�ϐ�

	// �p�u���b�N�֐�
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	// �v���C�x�[�g�ϐ�
	static const uint32_t FrameCount = 2;	// �t���[���o�b�t�@��

	HINSTANCE m_hInst;	// �C���X�^���X�n���h��
	HWND	  m_hWnd;	// �E�B���h�E�n���h��
	uint32_t  m_Width;	// �E�B���h�E�̉���
	uint32_t  m_Height;	// �E�B���h�E�̏c��

	ComPtr<ID3D12Device>			  m_pDevice;					// �f�o�C�X
	ComPtr<ID3D12CommandQueue>		  m_pQueue;						// �R�}���h�L���[
	ComPtr<IDXGISwapChain3>			  m_pSwapChain;					// �X���b�v�`�F�C��
	ComPtr<ID3D12Resource>			  m_pColorBuffer[FrameCount];	// �J���[�o�b�t�@
	ComPtr<ID3D12CommandAllocator>	  m_pCmdAllocator[FrameCount];	// �R�}���h�A���P�[�^
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;					// �R�}���h���X�g
	ComPtr<ID3D12DescriptorHeap>	  m_pHeapRTV;					// �f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Fence>				  m_pFence;						// �t�F���X
	ComPtr<ID3D12DescriptorHeap>	  m_pHeapCBV;					// �f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource>			  m_pVB;						// ���_�o�b�t�@
	ComPtr<ID3D12Resource>            m_pIB;						// �C���f�b�N�X�o�b�t�@�ł�.
	ComPtr<ID3D12Resource>			  m_pCB[FrameCount*2];			// �萔�o�b�t�@
	ComPtr<ID3D12RootSignature>		  m_pRootSignature;				// ���[�g�V�O�l�`��
	ComPtr<ID3D12PipelineState>		  m_pPSO;						// �p�C�v���C���X�e�[�g

	HANDLE							  m_FenceEvent;					// �t�F���X�C�x���g
	uint64_t						  m_FenceCounter[FrameCount];	// �t�F���X�J�E���g
	uint32_t						  m_FrameIndex;					// �t���[���ԍ�
	D3D12_CPU_DESCRIPTOR_HANDLE		  m_HandleRTV[FrameCount];		// CPU�f�B�X�N���v�^�i�����_�[�^�[�Q�b�g�r���[�j
	D3D12_VERTEX_BUFFER_VIEW		  m_VBV;						// ���_�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW			  m_IBV;                        // �C���f�b�N�X�o�b�t�@�r���[�ł�.
	D3D12_VIEWPORT					  m_Viewport;					// �r���[�|�[�g
	D3D12_RECT						  m_Scissor;					// �V�U�[��`
	ConstantBufferView<Transform>	  m_CBV[FrameCount*2];			// �萔�o�b�t�@�r���[
	float							  m_RotateAngle;				// ��]�p

	// �v���C�x�[�g�֐�
    bool InitApp();
    void TermApp();
    bool InitWnd();
    void TermWnd();
    void MainLoop();
    bool InitD3D();
    void TermD3D();
    void Render();
    void WaitGpu();
    void Present(uint32_t interval);
    bool OnInit();
    void OnTerm();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};