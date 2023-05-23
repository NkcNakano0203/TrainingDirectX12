#include <App.h>
#include "ResourceUploadBatch.h"
#include "DDSTextureLoader.h"
//#include "VertexTypes.h"
#include "FileUtil.h"
#include <cassert>

#include <iostream>

namespace
{
	const auto ClassName = TEXT("SampleWindowClass");    //!< �E�B���h�E�N���X��
}

// �R���X�g���N�^
App::App(uint32_t width, uint32_t height) :
	m_hInst(nullptr),
	m_hWnd(nullptr),
	m_Width(width),
	m_Height(height),
	m_FrameIndex(0)
{
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pColorBuffer[i] = nullptr;
		m_pCmdAllocator[i] = nullptr;
		m_FenceCounter[i] = 0;
	}
}

// �f�X�g���N�^
App::~App()
{

}

// ���s
void App::Run()
{
	if (InitApp())
	{
		MainLoop();
	}

	TermApp();
}

// ����������
bool App::InitApp()
{
	// �E�B���h�E�̏�����
	if (!InitWnd())
	{
		return false;
	}

	// Direct3D 12�̏�����
	if (!InitD3D())
	{
		return false;
	}

	if (!OnInit())
	{
		std::cout << "ouch";
		return false;
	}

	// ����I��
	return true;
}

// �I������
void App::TermApp()
{
	OnTerm();

	// Direct3D 12�̏I������
	TermD3D();

	// �E�B���h�E�̏I������
	TermWnd();
}

// �E�B���h�E�̏���������
bool App::InitWnd()
{
	// �C���X�^���X�n���h�����擾
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
	{
		return false;
	}

	// �E�B���h�E�̐ݒ�
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

	// �E�B���h�E�̓o�^.
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// �C���X�^���X�n���h���ݒ�
	m_hInst = hInst;

	// �E�B���h�E�̃T�C�Y��ݒ�
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// �E�B���h�E�T�C�Y�𒲐�
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// �E�B���h�E�𐶐�
	m_hWnd = CreateWindowEx(
		0,
		ClassName,
		TEXT("�������傤�E�B���h�E"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		m_hInst,
		nullptr);

	if (m_hWnd == nullptr)
	{
		return false;
	}

	// �E�B���h�E��\��
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// �E�B���h�E���X�V
	UpdateWindow(m_hWnd);

	// �E�B���h�E�Ƀt�H�[�J�X��ݒ�
	SetFocus(m_hWnd);

	// ����I��
	return true;
}

// �E�B���h�E�̏I������
void App::TermWnd()
{
	// �E�B���h�E�̓o�^������
	if (m_hInst != nullptr)
	{
		UnregisterClass(ClassName, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}

// Direct3D�̏���������
bool App::InitD3D()
{
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debug;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));

		// �f�o�b�O���C���[��L����
		if (SUCCEEDED(hr))
		{
			debug->EnableDebugLayer();
		}
	}
#endif

	// �f�o�C�X�̐���
	auto hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(m_pDevice.GetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	// �R�}���h�L���[�̐���
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
	}

	// �X���b�v�`�F�C���̐���
	{
		// DXGI�t�@�N�g���[�̐���
		ComPtr<IDXGIFactory4> pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		// �X���b�v�`�F�C���̐ݒ�
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// �X���b�v�`�F�C���̐���
		ComPtr<IDXGISwapChain> pSwapChain;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, pSwapChain.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// IDXGISwapChain3 ���擾
		hr = pSwapChain.As(&m_pSwapChain);
		if (FAILED(hr))
		{
			return false;
		}

		// �o�b�N�o�b�t�@�ԍ����擾
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		// �s�v�ɂȂ����̂ŉ��
		pFactory.Reset();
		pSwapChain.Reset();
	}

	// �R�}���h�A���P�[�^�̐���
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_pCmdAllocator[i].GetAddressOf()));
			if (FAILED(hr))
			{
				return false;
			}
		}
	}

	// �R�}���h���X�g�̐���
	{
		hr = m_pDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_pCmdAllocator[m_FrameIndex].Get(),
			nullptr,
			IID_PPV_ARGS(m_pCmdList.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
	}

	// �����_�[�^�[�Q�b�g�r���[�̐���
	{
		// �f�B�X�N���v�^�q�[�v�̐ݒ�
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		// �f�B�X�N���v�^�q�[�v�𐶐�
		hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pHeapRTV.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_pDevice
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pColorBuffer[i].GetAddressOf()));
			if (FAILED(hr))
			{
				return false;
			}

			D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			viewDesc.Texture2D.PlaneSlice = 0;

			// �����_�[�^�[�Q�b�g�r���[�̐���
			m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);

			m_HandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	// �[�x�X�e���V���o�b�t�@�̐���
	{
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Alignment = 0;
		resDesc.Width = m_Width;
		resDesc.Height = m_Height;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = DXGI_FORMAT_D32_FLOAT;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0;
		clearValue.DepthStencil.Stencil = 0;

		hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(m_pDepthBuffer.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		// �f�B�X�N���v�^�q�[�v�̐ݒ�.
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;

		hr = m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pHeapDSV.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		auto handle = m_pHeapDSV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_pDevice->CreateDepthStencilView(m_pDepthBuffer.Get(), &viewDesc, handle);

		m_HandleDSV = handle;
	}

	// �t�F���X�̐���.
	{
		// �t�F���X�J�E���^�[�����Z�b�g
		for (auto i = 0u; i < FrameCount; ++i)
		{
			m_FenceCounter[i] = 0;
		}

		// �t�F���X�̐���
		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(m_pFence.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		m_FenceCounter[m_FrameIndex]++;

		// �C�x���g�̐���
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_FenceEvent == nullptr)
		{
			return false;
		}
	}
	// �R�}���h���X�g�����
	//m_pCmdList->Close();

	return true;
}

// Direct3D�̏I������
void App::TermD3D()
{
	// GPU�����̊�����ҋ@
	WaitGpu();

	// �C�x���g�j��
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	// �t�F���X�j��
	m_pFence.Reset();

	// �����_�[�^�[�Q�b�g�r���[�̔j��
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pColorBuffer[i].Reset();
	}

	// �[�x�X�e���V���r���[�̔j��.
	m_pHeapDSV.Reset();
	m_pDepthBuffer.Reset();

	// �R�}���h���X�g�̔j��
	m_pCmdList.Reset();

	// �R�}���h�A���P�[�^�̔j��
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pCmdAllocator[i].Reset();
	}

	// �X���b�v�`�F�C���̔j��
	m_pSwapChain.Reset();

	// �R�}���h�L���[�̔j��
	m_pQueue.Reset();

	// �f�o�C�X�̔j��
	m_pDevice.Reset();
}

// ���������̏���
bool App::OnInit()
{
	// ���b�V�������[�h
	{
		std::wstring path;
		if (!SearchFilePath(L"res/teapot/teapot.obj", path))
		{
			return false;
		}
		//m_Meshes.resize(1);
		//m_Materials.resize(1);
		if (!LoadMesh(path.c_str(), m_Meshes, m_Materials))
		{
			std::cout << "fal";
			return false;
		}
		//std::cout << "Piyo";

		// ���̃T���v���ł́C���b�V����1�݂̂Ƃ��܂�.
		assert(m_Meshes.size() == 1);
	}
	// ���_�o�b�t�@�̐���
	{
		// ���_�f�[�^
		//DirectX::VertexPositionTexture vertices[] =
		//{
		//	 DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)),
		//	 DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)),
		//	 DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f,-1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)),
		//	 DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f,-1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f))
		//};
		auto size = sizeof(MeshVertex) * m_Meshes[0].Vertices.size();
		auto vertices = m_Meshes[0].Vertices.data();

		// �q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type					= D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask		= 1;
		prop.VisibleNodeMask		= 1;

		// ���\�[�X�̐ݒ�
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment			= 0;
		desc.Width				= size;
		desc.Height				= 1;
		desc.DepthOrArraySize	= 1;
		desc.MipLevels			= 1;
		desc.Format				= DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count	= 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags				= D3D12_RESOURCE_FLAG_NONE;

		// ���\�[�X�𐶐�
		auto hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pVB.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		// �}�b�s���O
		void* ptr = nullptr;
		hr = m_pVB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}

		// ���_�f�[�^���}�b�s���O��ɐݒ�
		memcpy(ptr, vertices, size);

		// �}�b�s���O����
		m_pVB->Unmap(0, nullptr);

		// ���_�o�b�t�@�r���[�̐ݒ�
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = static_cast<UINT>(size);
		//m_VBV.StrideInBytes = static_cast<UINT>(sizeof(DirectX::VertexPositionTexture));
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(MeshVertex));
	}

	// �C���f�b�N�X�o�b�t�@�̐���
	{
		//uint32_t indices[] = { 0,1,2,0,2,3 };
		auto size = sizeof(uint32_t) * m_Meshes[0].Indices.size();
		auto indices = m_Meshes[0].Indices.data();

		// �q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		//���\�[�X�̐ݒ�
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// ���\�[�X�𐶐�
		auto hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pIB.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		// �}�b�s���O����
		void* ptr = nullptr;
		hr = m_pIB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}

		// �C���f�b�N�X�f�[�^���}�b�s���O��ɐݒ�
		memcpy(ptr, indices, size);
		
		// �}�b�s���O����
		m_pIB->Unmap(0, nullptr);

		// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�
		m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();
		m_IBV.Format = DXGI_FORMAT_R32_UINT;
		m_IBV.SizeInBytes = static_cast<UINT>(size);
	}

	// CBV�^SRV�^UAV�p�f�B�X�N���v�^�q�[�v�̐���.
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 2 * FrameCount;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		auto hr = m_pDevice->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(m_pHeapCBV.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
	}

	// �萔�o�b�t�@�̐���
	{
		// �q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// ���\�[�X�̐ݒ�
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = sizeof(Transform);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto incrementSize = m_pDevice
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (auto i = 0; i < FrameCount; ++i)
		{
			// ���\�[�X����
			auto hr = m_pDevice->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_pCB[i].GetAddressOf()));
			if (FAILED(hr))
			{
				return false;
			}

			auto address = m_pCB[i]->GetGPUVirtualAddress();
			auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
			auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();

			handleCPU.ptr += incrementSize * i;
			handleGPU.ptr += incrementSize * i;

			// �萔�o�b�t�@�r���[�̐ݒ�
			m_CBV[i].HandleCPU = handleCPU;
			m_CBV[i].HandleGPU = handleGPU;
			m_CBV[i].Desc.BufferLocation = address;
			m_CBV[i].Desc.SizeInBytes = sizeof(Transform);

			// �萔�o�b�t�@�r���[�𐶐�
			m_pDevice->CreateConstantBufferView(&m_CBV[i].Desc, handleCPU);

			// �}�b�s���O
			hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));
			if (FAILED(hr))
			{
				return false;
			}

			auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
			auto targetPos = DirectX::XMVectorZero();
			auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			auto fovY = DirectX::XMConvertToRadians(37.5f);
			auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

			// �ϊ��s��̐ݒ�
			m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity();
			m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
			m_CBV[i].pBuffer->Proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
		}
	}

	// ���[�g�V�O�j�`���̐���
	{
		auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// ���[�g�p�����[�^�̐ݒ�
		D3D12_ROOT_PARAMETER param[2] = {};
		param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param[0].Descriptor.ShaderRegister = 0;
		param[0].Descriptor.RegisterSpace = 0;
		param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		D3D12_DESCRIPTOR_RANGE range = {};
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.NumDescriptors = 1;
		range.BaseShaderRegister = 0;
		range.RegisterSpace = 0;
		range.OffsetInDescriptorsFromTableStart = 0;

		param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[1].DescriptorTable.NumDescriptorRanges = 1;
		param[1].DescriptorTable.pDescriptorRanges = &range;
		param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// �X�^�e�B�b�N�T���v���[�̐ݒ�
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		sampler.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
		sampler.MaxAnisotropy = 1;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = -D3D12_FLOAT32_MAX;
		sampler.MaxLOD = +D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// ���[�g�V�O�j�`���̐ݒ�
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = 2;
		desc.NumStaticSamplers = 1;
		desc.pParameters = param;
		desc.pStaticSamplers = &sampler;
		desc.Flags = flag;

		ComPtr<ID3DBlob> pBlob;
		ComPtr<ID3DBlob> pErrorBlob;

		// �V���A���C�Y
		auto hr = D3D12SerializeRootSignature(
			&desc,
			D3D_ROOT_SIGNATURE_VERSION_1_0,
			pBlob.GetAddressOf(),
			pErrorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// ���[�g�V�O�j�`���𐶐�
		hr = m_pDevice->CreateRootSignature(
			0,
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			IID_PPV_ARGS(m_pRootSignature.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
	}

	// �p�C�v���C���X�e�[�g�̐���
	{
		// ���̓��C�A�E�g�̐ݒ�
		D3D12_INPUT_ELEMENT_DESC elements[2];
		elements[0].SemanticName = "POSITION";
		elements[0].SemanticIndex = 0;
		elements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elements[0].InputSlot = 0;
		elements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		elements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elements[0].InstanceDataStepRate = 0;

		elements[1].SemanticName = "TEXCOORD";
		elements[1].SemanticIndex = 0;
		elements[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		elements[1].InputSlot = 0;
		elements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		elements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elements[1].InstanceDataStepRate = 0;

		// ���X�^���C�U�[�X�e�[�g�̐ݒ�
		D3D12_RASTERIZER_DESC descRS;
		descRS.FillMode = D3D12_FILL_MODE_SOLID;
		descRS.CullMode = D3D12_CULL_MODE_NONE;
		descRS.FrontCounterClockwise = FALSE;
		descRS.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		descRS.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		descRS.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		descRS.DepthClipEnable = FALSE;
		descRS.MultisampleEnable = FALSE;
		descRS.AntialiasedLineEnable = FALSE;
		descRS.ForcedSampleCount = 0;
		descRS.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�
		D3D12_RENDER_TARGET_BLEND_DESC descRTBS = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		// �u�����h�X�e�[�g�̐ݒ�
		D3D12_BLEND_DESC descBS;
		descBS.AlphaToCoverageEnable = FALSE;
		descBS.IndependentBlendEnable = FALSE;
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			descBS.RenderTarget[i] = descRTBS;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		std::wstring vsPath;
		std::wstring psPath;

		if (!SearchFilePath(L"SimpleTexVS.cso", vsPath))
		{
			return false;
		}

		if (!SearchFilePath(L"SimpleTexPS.cso", psPath))
		{
			return false;
		}

		// ���_�V�F�[�_�ǂݍ���.
		auto hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// �s�N�Z���V�F�[�_�ǂݍ���.
		hr = D3DReadFileToBlob(psPath.c_str(), pPSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// �p�C�v���C���X�e�[�g�̐ݒ�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		//desc.InputLayout = { elements, _countof(elements) };
		desc.InputLayout = MeshVertex::InputLayout;
		desc.pRootSignature = m_pRootSignature.Get();
		desc.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
		desc.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
		desc.RasterizerState = descRS;
		desc.BlendState = descBS;
		desc.DepthStencilState.DepthEnable = FALSE;
		desc.DepthStencilState.StencilEnable = FALSE;
		desc.SampleMask = UINT_MAX;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		// �p�C�v���C���X�e�[�g�𐶐�
		hr = m_pDevice->CreateGraphicsPipelineState(
			&desc,
			IID_PPV_ARGS(m_pPSO.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
	}

	// �e�N�X�`���̐���
	{
		// �t�@�C���p�X������
		std::wstring texturePath;
		if (!SearchFilePath(L"res/teapot/default.dds", texturePath))
		{
			return false;
		}

		DirectX::ResourceUploadBatch batch(m_pDevice.Get());
		batch.Begin();

		// ���\�[�X�𐶐�.
		auto hr = DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(),
			batch,
			texturePath.c_str(),
			m_Texture.pResource.GetAddressOf(),
			true);
		if (FAILED(hr))
		{
			return false;
		}

		// �R�}���h�����s
		auto future = batch.End(m_pQueue.Get());

		// �R�}���h�̊�����ҋ@����.
		future.wait();

		// �C���N�������g�T�C�Y���擾.
		auto incrementSize = m_pDevice
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// CPU�f�B�X�N���v�^�n���h����GPU�f�B�X�N���v�^�n���h�����f�B�X�N���v�^�q�[�v����擾.
		auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
		auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();

		// �e�N�X�`���Ƀf�B�X�N���v�^�����蓖�Ă�
		handleCPU.ptr += incrementSize * 2;
		handleGPU.ptr += incrementSize * 2;

		m_Texture.HandleCPU = handleCPU;
		m_Texture.HandleGPU = handleGPU;

		// �e�N�X�`���̍\���ݒ���擾
		auto textureDesc = m_Texture.pResource->GetDesc();

		// �V�F�[�_���\�[�X�r���[�̐ݒ�.
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Format = textureDesc.Format;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.Texture2D.MostDetailedMip = 0;
		viewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		viewDesc.Texture2D.PlaneSlice = 0;
		viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		// �V�F�[�_���\�[�X�r���[�𐶐�.
		m_pDevice->CreateShaderResourceView(
			m_Texture.pResource.Get(), &viewDesc, handleCPU);
	}

	// �r���[�|�[�g�ƃV�U�[��`�̐ݒ�
	{
		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;
		m_Viewport.Width = static_cast<float>(m_Width);
		m_Viewport.Height = static_cast<float>(m_Height);
		m_Viewport.MinDepth = 0.0f;
		m_Viewport.MaxDepth = 1.0f;

		m_Scissor.left = 0;
		m_Scissor.right = m_Width;
		m_Scissor.top = 0;
		m_Scissor.bottom = m_Height;
	}

	return true;
}

// �I�����̏���
void App::OnTerm()
{
	for (auto i = 0; i < FrameCount; ++i)
	{
		if (m_pCB[i].Get() != nullptr)
		{
			m_pCB[i]->Unmap(0, nullptr);
			memset(&m_CBV[i], 0, sizeof(m_CBV[i]));
		}
		m_pCB[i].Reset();
	}

	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		m_Meshes[i].Vertices.clear();
		m_Meshes[i].Indices.clear();
	}
	m_Meshes.clear();
	m_Materials.clear();

	m_pIB.Reset();
	m_pVB.Reset();
	m_pPSO.Reset();
	m_pHeapCBV.Reset();

	m_VBV.BufferLocation = 0;
	m_VBV.SizeInBytes = 0;
	m_VBV.StrideInBytes = 0;

	m_IBV.BufferLocation = 0;
	m_IBV.Format = DXGI_FORMAT_UNKNOWN;
	m_IBV.SizeInBytes = 0;

	m_pRootSignature.Reset();

	m_Texture.pResource.Reset();
	m_Texture.HandleCPU.ptr = 0;
	m_Texture.HandleGPU.ptr = 0;
}

// ���C�����[�v
void App::MainLoop()
{
	MSG msg = {};

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
}

// �`�揈��
void App::Render()
{
	// �X�V����
	{
		m_RotateAngle += 0.015f;
		m_CBV[m_FrameIndex].pBuffer->World = DirectX::XMMatrixRotationY(m_RotateAngle);
	}

	// �R�}���h�̋L�^���J�n
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr);

	// ���\�[�X�o���A�̐ݒ�
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// ���\�[�X�o���A
	m_pCmdList->ResourceBarrier(1, &barrier);

	// �����_�[�Q�b�g�̐ݒ�
	m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

	// �N���A�J���[�̐ݒ�
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	// �����_�[�^�[�Q�b�g�r���[���N���A
	m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

	// �`�揈��
	{
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
		m_pCmdList->SetDescriptorHeaps(1, m_pHeapCBV.GetAddressOf());
		m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex].Desc.BufferLocation);
		m_pCmdList->SetGraphicsRootDescriptorTable(1, m_Texture.HandleGPU);
		m_pCmdList->SetPipelineState(m_pPSO.Get());

		m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCmdList->IASetVertexBuffers(0, 1, &m_VBV);
		m_pCmdList->IASetIndexBuffer(&m_IBV);
		m_pCmdList->RSSetViewports(1, &m_Viewport);
		m_pCmdList->RSSetScissorRects(1, &m_Scissor);

		//m_pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		auto count = static_cast<uint32_t>(m_Meshes[0].Indices.size());
		m_pCmdList->DrawIndexedInstanced(count, 1, 0, 0, 0);
	}

	// ���\�[�X�o���A�̐ݒ�
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// ���\�[�X�o���A
	m_pCmdList->ResourceBarrier(1, &barrier);

	// �R�}���h�̋L�^���I��
	m_pCmdList->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// ��ʂɕ\��
	Present(1);
}

// ��ʂɕ\�����C���̃t���[���̏������s��
void App::Present(uint32_t interval)
{
	// ��ʂɕ\��
	m_pSwapChain->Present(interval, 0);

	// �V�O�i������
	const auto currentValue = m_FenceCounter[m_FrameIndex];
	m_pQueue->Signal(m_pFence.Get(), currentValue);

	// �o�b�N�o�b�t�@�ԍ����X�V
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// ���̃t���[���̕`�揀�����܂��ł���Αҋ@����
	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
	{
		m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}

	// ���̃t���[���̃t�F���X�J�E���^�[�𑝂₷
	m_FenceCounter[m_FrameIndex] = currentValue + 1;
}

// GPU�̏���������ҋ@
void App::WaitGpu()
{
	assert(m_pQueue != nullptr);
	assert(m_pFence != nullptr);
	assert(m_FenceEvent != nullptr);

	// �V�O�i������
	m_pQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);

	// �������ɃC�x���g��ݒ肷��
	m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);

	// �ҋ@����
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	// �J�E���^�[�𑝂₷
	m_FenceCounter[m_FrameIndex]++;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;

	default:
	{

	}
	break;
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}
