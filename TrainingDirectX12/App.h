#pragma once
// インクルード
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

class App
{
public:
	// パブリック変数

	// パブリック関数
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	// プライベート変数
	static const uint32_t FrameCount = 2;	// フレームバッファ数

	HINSTANCE m_hInst;	// インスタンスハンドル
	HWND m_hWnd;		// ウィンドウハンドル
	uint32_t m_Width;	// ウィンドウの横幅
	uint32_t m_Height;	// ウィンドウの縦幅

	ID3D12Device* m_pDevice;							// デバイス
	ID3D12CommandQueue* m_pQueue;						// コマンドキュー
	IDXGISwapChain3* m_pSwapChain;						// スワップチェイン
	ID3D12Resource* m_pColorBuffer[FrameCount];			// カラーバッファ
	ID3D12CommandAllocator* m_pCmdAllocator[FrameCount];// コマンドアロケータ
	ID3D12GraphicsCommandList* m_pCmdList;				// コマンドリスト
	ID3D12DescriptorHeap* m_pHeapRTV;					// ディスクリプタヒープ
	ID3D12Fence* m_pFence;								// フェンス
	HANDLE m_FenceEvent;								// フェンスイベント
	uint64_t m_FenceCounter[FrameCount];				// フェンスカウント
	uint32_t m_FrameIndex;								// フレーム番号
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];// CPUディスクリプタ（レンダーターゲットビュー）

	// プライベート関数
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