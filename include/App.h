#pragma once

// インクルード
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

// Tranform構造体
struct alignas(256) Transform
{
	DirectX::XMMATRIX   World;	// ワールド行列
	DirectX::XMMATRIX   View;	// ビュー行列
	DirectX::XMMATRIX   Proj;	// 射影行列
};

template<typename T> struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;		// 定数バッファの構成設定
	D3D12_CPU_DESCRIPTOR_HANDLE     HandleCPU;	// CPUディスクリプタハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE     HandleGPU;	// GPUディスクリプタハンドル
	T* pBuffer;									// バッファ先頭へのポインタ
};

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
	HWND	  m_hWnd;	// ウィンドウハンドル
	uint32_t  m_Width;	// ウィンドウの横幅
	uint32_t  m_Height;	// ウィンドウの縦幅

	ComPtr<ID3D12Device>			  m_pDevice;					// デバイス
	ComPtr<ID3D12CommandQueue>		  m_pQueue;						// コマンドキュー
	ComPtr<IDXGISwapChain3>			  m_pSwapChain;					// スワップチェイン
	ComPtr<ID3D12Resource>			  m_pColorBuffer[FrameCount];	// カラーバッファ
	ComPtr<ID3D12CommandAllocator>	  m_pCmdAllocator[FrameCount];	// コマンドアロケータ
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;					// コマンドリスト
	ComPtr<ID3D12DescriptorHeap>	  m_pHeapRTV;					// ディスクリプタヒープ
	ComPtr<ID3D12Fence>				  m_pFence;						// フェンス
	ComPtr<ID3D12DescriptorHeap>	  m_pHeapCBV;					// ディスクリプタヒープ
	ComPtr<ID3D12Resource>			  m_pVB;						// 頂点バッファ
	ComPtr<ID3D12Resource>            m_pIB;						// インデックスバッファです.
	ComPtr<ID3D12Resource>			  m_pCB[FrameCount*2];			// 定数バッファ
	ComPtr<ID3D12RootSignature>		  m_pRootSignature;				// ルートシグネチャ
	ComPtr<ID3D12PipelineState>		  m_pPSO;						// パイプラインステート

	HANDLE							  m_FenceEvent;					// フェンスイベント
	uint64_t						  m_FenceCounter[FrameCount];	// フェンスカウント
	uint32_t						  m_FrameIndex;					// フレーム番号
	D3D12_CPU_DESCRIPTOR_HANDLE		  m_HandleRTV[FrameCount];		// CPUディスクリプタ（レンダーターゲットビュー）
	D3D12_VERTEX_BUFFER_VIEW		  m_VBV;						// 頂点バッファビュー
	D3D12_INDEX_BUFFER_VIEW			  m_IBV;                        // インデックスバッファビューです.
	D3D12_VIEWPORT					  m_Viewport;					// ビューポート
	D3D12_RECT						  m_Scissor;					// シザー矩形
	ConstantBufferView<Transform>	  m_CBV[FrameCount*2];			// 定数バッファビュー
	float							  m_RotateAngle;				// 回転角

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
    bool OnInit();
    void OnTerm();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};