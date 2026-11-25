#include "Renderer.h"
#include"Window.h"
#include<d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

/// <summary>
/// 初期化
/// </summary>
/// <param name="hWindow"></param>
/// <returns></returns>
bool Renderer::Initialize(HWND hWindow)
{
	//	Windowにあわせてスクリーンサイズを初期化
	RECT rc;
	GetClientRect(hWindow, &rc);
	screenWidth_ = rc.right - rc.left;
	screenHeight_ = rc.bottom - rc.top;

	initDeviceAndSwapChain(hWindow);

	initBackBuffer();

	CompileShader(L"VertexShader.hlsl", L"PixelShader.hlsl", DefaultShader);

	return true;
}

/// <summary>
/// デバイスとスワップチェインの作成
/// </summary>
/// <param name="hWindow"></param>
/// <returns></returns>
bool  Renderer::initDeviceAndSwapChain(HWND hWindow)
{
	//	デバイスとスワップチェインの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = backBufferNum_;
	sd.BufferDesc.Width = screenWidth_;
	sd.BufferDesc.Height = screenHeight_;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWindow;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#if defined(DEBUG)||defined(_DEBUG)
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT createDeviceFlags = 0;
#endif

	const D3D_DRIVER_TYPE DriverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	HRESULT hr;
	for (auto type : DriverTypes) 
	{
		//	ハードウェアデバイスを作成
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr, type, nullptr, createDeviceFlags,
			pFeatureLevels_, FEATURE_LEVELS_NUM, D3D11_SDK_VERSION, &sd,
			&pSwapChain_, &pD3DDevice_, &featureLevelsSupported_, &pImmediateContext_);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	if (FAILED(hr)) return false;

	return true;
}

/// <summary>
///	バックバッファ設定
/// </summary>
/// <returns></returns>
bool Renderer::initBackBuffer()
{
	HRESULT hr;

	//	スワップチェインから最初のバックバッファを取得
	ID3D11Texture2D* pBackBuffer;
	hr = pSwapChain_->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBuffer);
	if (FAILED(hr))
	{
		//TRACE("InitBackBuffer g_pSwapChain->GetBuffer(%0x08)n", hr);	//	失敗
		return false;

	}

	//	バックバッファの情報
	D3D11_TEXTURE2D_DESC descBackBuffer;
	pBackBuffer->GetDesc(&descBackBuffer);

	//	バックバッファの描画ターゲットビューを作る
	hr = pD3DDevice_->CreateRenderTargetView(
		pBackBuffer,
		nullptr,
		&pRenderTargetView_);
	//	解放
	DX_SAFE_RELEASE(pBackBuffer);
	
	//	ビューポートの設定
	viewPort_[0].TopLeftX = 0.0f;
	viewPort_[0].TopLeftY = 0.0f;
	viewPort_[0].Width = static_cast<float>(screenWidth_);
	viewPort_[0].Height = static_cast<float>(screenHeight_);
	viewPort_[0].MinDepth = 0.0f;
	viewPort_[0].MaxDepth = 1.0f;
	pImmediateContext_->RSSetViewports(1, &viewPort_[0]);

	return true;

}

/// <summary>
///	シェーダーコンパイルと入力レイアウトオブジェクトの作成
/// </summary>
/// <param name="vsPath"></param>
/// <param name="psPath"></param>
/// <param name="outShader"></param>
/// <returns></returns>
bool Renderer::CompileShader(const WCHAR* vsPath, const WCHAR* psPath, Shader& outShader)
{
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* errBlob = nullptr;
	auto pDevice = GetDevice();

	//	シェーダーコンパイル
	auto hr = D3DCompileFromFile(
		vsPath,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob,
		&errBlob
	);
	if (FAILED(hr))return false;

	//	頂点シェーダ作成（シェーダオブジェクト作成）
	ID3D11VertexShader* pVertexShader = nullptr;
	hr = pDevice->CreateVertexShader(
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		nullptr,
		&pVertexShader
	);
	if (FAILED(hr)) return false;

	//	入力レイアウトオブジェクト作成
	ID3D11InputLayout* pInputLayout = nullptr;
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	hr = pDevice->CreateInputLayout(
		layout,
		_countof(layout),
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		&pInputLayout
	);
	if (FAILED(hr)) return false;

	//	ピクセルシェーダー作成
	ID3DBlob* psBlob = nullptr;
	hr = D3DCompileFromFile(
		psPath,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errBlob
	);

	ID3D11PixelShader* pPixelShader = nullptr;
	hr = pDevice->CreatePixelShader(
		psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(),
		nullptr,
		&pPixelShader
	);
	if (FAILED(hr)) return false;

	outShader.pVertexShader = pVertexShader;
	outShader.pPixelShader = pPixelShader;
	outShader.pInputLayout = pInputLayout;

	return true;
}


//	描画処理
void Renderer::Draw()
{
	if (!pImmediateContext_ || !pRenderTargetView_)	return;

	pImmediateContext_->OMSetRenderTargets(1, &pRenderTargetView_, nullptr);

	//	青でクリア
	float color[] = { 0.f,0.f,1.f,0.f };
	pImmediateContext_->ClearRenderTargetView(pRenderTargetView_, color);

	pImmediateContext_->IASetInputLayout(DefaultShader.pInputLayout);
	pImmediateContext_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pImmediateContext_->VSSetShader(DefaultShader.pVertexShader, nullptr, 0);
	pImmediateContext_->PSSetShader(DefaultShader.pPixelShader, nullptr, 0);

}

//	バッファの変換
void Renderer::Swap()
{
	//	バックバッファの表示
	HRESULT hr = pSwapChain_->Present(0,0);
	if (FAILED(hr))
	{
		return;

	}
}

//	解放処理
void Renderer::Terminate()
{
	//	デバイスステートのクリア
	if (pImmediateContext_)	pImmediateContext_->ClearState();

	//	スワップチェインをウィンドウモードにする
	if (pSwapChain_)	pSwapChain_->SetFullscreenState(FALSE, nullptr);

	//	取得したインターフェイスの解放
	DX_SAFE_RELEASE(pRenderTargetView_);
	DX_SAFE_RELEASE(pSwapChain_);

	DX_SAFE_RELEASE(pImmediateContext_);
	DX_SAFE_RELEASE(pD3DDevice_);
}