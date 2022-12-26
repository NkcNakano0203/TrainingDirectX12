#pragma once
// �C���N���[�h
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

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
	HWND m_hWnd;		// �E�B���h�E�n���h��
	uint32_t m_Width;	// �E�B���h�E�̉���
	uint32_t m_Height;	// �E�B���h�E�̏c��

	ID3D12Device* m_pDevice;							// �f�o�C�X
	ID3D12CommandQueue* m_pQueue;						// �R�}���h�L���[
	IDXGISwapChain3* m_pSwapChain;						// �X���b�v�`�F�C��
	ID3D12Resource* m_pColorBuffer[FrameCount];			// �J���[�o�b�t�@
	ID3D12CommandAllocator* m_pCmdAllocator[FrameCount];// �R�}���h�A���P�[�^
	ID3D12GraphicsCommandList* m_pCmdList;				// �R�}���h���X�g
	ID3D12DescriptorHeap* m_pHeapRTV;					// �f�B�X�N���v�^�q�[�v
	ID3D12Fence* m_pFence;								// �t�F���X
	HANDLE m_FenceEvent;								// �t�F���X�C�x���g
	uint64_t m_FenceCounter[FrameCount];				// �t�F���X�J�E���g
	uint32_t m_FrameIndex;								// �t���[���ԍ�
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];// CPU�f�B�X�N���v�^�i�����_�[�^�[�Q�b�g�r���[�j

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

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};