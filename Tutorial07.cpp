//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// This application demonstrates texturing
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729724.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"

//Own
#include "Object.h"
#include "CameraTest.h"
#include "VectorMaths.h"
#include <functional> 
#include <vector>
#include "Target.h"
#include <string>
//const float pi = 3.14159265359f;
//float DegtoRag(float v)
//{
	//float y = v * pi / 180;
	//return y;
//}
#define RenderSelector true
#define SelectorDistance 5.0f
#define speed -0.15f
#define FPS 60
#define targetDelta (1000/FPS)
#define MouseSensitivity 0.035f

#define WindowWidth 800
#define WindowHeight 600
float currentDelta = 0.0f;
float ScaleFactor = 0.0f;
float frameEnd = 0.0f;

float timeLast = 0;
using namespace DirectX;



//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct CBNeverChanges
{
	XMMATRIX mView;
};

struct CBChangeOnResize
{
	XMMATRIX mProjection;
};

struct CBChangesEveryFrame
{
	XMMATRIX mWorld;
	XMFLOAT4 vMeshColor;
	XMMATRIX view2;
	XMMATRIX mProjection2; //Possibly not needed and our view2 matrix;
	//Lighting variables from tutorial 06
	XMFLOAT4 vLightDir[2];
	XMFLOAT4 vLightColor[2];
	XMFLOAT4 vOutputColor;
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = nullptr;
HWND                                g_hWnd = nullptr;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = nullptr;
ID3D11Device1*                      g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*                g_pImmediateContext = nullptr;
ID3D11DeviceContext1*               g_pImmediateContext1 = nullptr;
IDXGISwapChain*                     g_pSwapChain = nullptr;
IDXGISwapChain1*                    g_pSwapChain1 = nullptr;
ID3D11RenderTargetView*             g_pRenderTargetView = nullptr;
ID3D11Texture2D*                    g_pDepthStencil = nullptr;
ID3D11DepthStencilView*             g_pDepthStencilView = nullptr;
ID3D11VertexShader*                 g_pVertexShader = nullptr;
ID3D11PixelShader*                  g_pPixelShader = nullptr;
ID3D11PixelShader*                  g_pPixelShaderSolid = nullptr; //Lighting Shader
ID3D11InputLayout*                  g_pVertexLayout = nullptr;
ID3D11Buffer*                       g_pVertexBuffer = nullptr;
ID3D11Buffer*                       g_pIndexBuffer = nullptr;
ID3D11Buffer*                       g_pCBNeverChanges = nullptr;
ID3D11Buffer*                       g_pCBChangeOnResize = nullptr;
ID3D11Buffer*                       g_pCBChangesEveryFrame = nullptr;
ID3D11ShaderResourceView*           g_pTextureRV = nullptr;
ID3D11SamplerState*                 g_pSamplerLinear = nullptr;
XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
//XMFLOAT4                            g_vMeshColor(0.7f, 0.7f, 0.7f, 1.0f);
XMFLOAT4 g_vMeshColor(0.15f, 0.15f, 0.15f, 1.0f);


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
DWORD currentTime = timeGetTime();
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
			currentTime = timeGetTime();
			if ((currentTime-timeLast)<(1000/FPS))
			{
				Sleep(currentTime - timeLast);
			}
			timeLast = currentTime;
			frameEnd = timeGetTime();
			currentDelta = (currentTime - frameEnd);
			ScaleFactor = (currentDelta / targetDelta);
		}

	}

	CleanupDevice();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, WindowWidth, WindowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 7",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}
ID3D11ShaderResourceView* Wood;

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tutorial07.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	 //Lighting Data sent through to shader file
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tutorial07.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Compile the pixel shader
	pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tutorial07.fx", "PSSolid", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShaderSolid);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;



	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Create index buffer
	// Create vertex buffer
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffers
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBNeverChanges);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBNeverChanges);
	if (FAILED(hr))
		return hr;

	bd.ByteWidth = sizeof(CBChangeOnResize);
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangeOnResize);
	if (FAILED(hr))
		return hr;

	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame);
	if (FAILED(hr))
		return hr;

	// Load the Texture
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"seafloor.dds", nullptr, &g_pTextureRV);
	if (FAILED(hr))
		return hr;

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;
	CreateDDSTextureFromFile(g_pd3dDevice, L"CustomWood.dds", nullptr, &Wood);
	// Initialize the world matrices
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	CBNeverChanges cbNeverChanges;
	cbNeverChanges.mView = XMMatrixTranspose(g_View);
	g_pImmediateContext->UpdateSubresource(g_pCBNeverChanges, 0, nullptr, &cbNeverChanges, 0, 0);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

	CBChangeOnResize yus;
	yus.mProjection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pCBChangeOnResize, 0, nullptr, &yus, 0, 0);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pTextureRV) g_pTextureRV->Release();
	if (g_pCBNeverChanges) g_pCBNeverChanges->Release();
	if (g_pCBChangeOnResize) g_pCBChangeOnResize->Release();
	if (g_pCBChangesEveryFrame) g_pCBChangesEveryFrame->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain1) g_pSwapChain1->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext1) g_pImmediateContext1->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice1) g_pd3dDevice1->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
char horizontal;
char vertical;
bool holding = false;
bool forwardtime;
float previousX;
float currentX;
float previousY;
float currentY;
bool rotating = true;
CameraTest CamBhoy;
std::vector<Object*> CreateRow(int length, int row)
{
	std::vector<Object*> Floor;
	Object *temp = new Object;
	for (int i = 0; i < length; i++)
	{
		temp = new Object;
		temp->SetPos(XMFLOAT3(i * 2, 0, row * 2));
		temp->SetTexture(Wood);
		Floor.push_back(temp);
	}
	delete temp;
	temp = NULL;
	return Floor;
}

struct Room
{
public:
	XMFLOAT3 Position = XMFLOAT3(0, 0, 0);
	std::vector<Object*> Floor;
	Object *temp = new Object;
	Room(int length, int width, int height)
	{
		//AB.clear();
		//AB.~vector();
		for (int i = 0; i < length - 1; i++) //Floor
		{
			std::vector<Object*> AB = CreateRow(width, i);
			Floor.insert(Floor.end(), AB.begin(), AB.end());
		}


		//for(int i=0;)
		//Floor += CreateRow(5, 0);
		//for (x; x < length;)
	//	{
		//	for (y;y<height;)
		//	{
				//temp = new Object();
				//temp->SetPos(XMFLOAT3(x * 2, 0, y*2));
				//Floor.push_back(temp);
			//}
		//}
		delete temp;
		temp = NULL;
		//for (int x = 0; x < width; x++)
	//	{
		//	for (int z = 0; z < length; z++)
		//	{
		//		temp = new Object();
		//		temp->SetPos(XMFLOAT3(x*2, 0, z*2));
		//		Floor.push_back(temp);
				//if (z==0||z==length)
				//{
				//	XMFLOAT3 tempos = temp->GetPos();
				//	temp = new Object();
				//	temp->SetPos(XMFLOAT3(tempos.x, tempos.y+2, tempos.z));
				//	Floor.push_back(temp);
				//}
			//	if (x==0 || x==length || z==0 || z==height)
			//	{
			//		temp = new Object();
		//			temp->SetPos(XMFLOAT3(x * 2, 2, z * 2));
		//			Floor.push_back(temp);
			//	}
		//	}

				//for (int y = 0; y < height; y++)
				//{
				//	temp = new Object();
				//	XMFLOAT3 tempos = Floor[Floor.size() - 1]->GetPos();
				//	temp->SetPos(XMFLOAT3(tempos.x, tempos.y + 2, tempos.z));
				//	Floor.push_back(temp);
				//}
			//temp = new Object();
			//XMFLOAT3 tempos = Floor[Floor.size() - 1]->GetPos();
			//temp->SetPos(XMFLOAT3(tempos.x, tempos.y + 2, tempos.z));
			//Floor.push_back(temp);

		//}
	//	XMFLOAT3 tempos = Floor[Floor.size() - 1]->GetPos();
	//	tempos += XMFLOAT3(tempos.x + 2, 0, tempos.z + 2);
	//	temp = new Object();
	//	temp->SetPos(tempos);
	//	delete temp;
		//temp = NULL;
	}
	Room()
	{

	}
};

/*Room(int length, int width, int height)
{
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < length; z++)
		{
			temp = new Object();
			temp->SetPos(XMFLOAT3(x * 2, 0, z * 2));
			Floor.push_back(temp);
		}
	}
	XMFLOAT3 tempos = Floor[Floor.size() - 1]->GetPos();
	tempos += XMFLOAT3(tempos.x + 2, 0, tempos.z + 2);
	temp = new Object();
	temp->SetPos(tempos);
	delete temp;
	temp = NULL;*/


void MouseMovement()
{
	if (currentX < previousX)
	{
		CamBhoy.Rotate(0, -MouseSensitivity, 0);
	}
	else if (currentX > previousX)
	{
		CamBhoy.Rotate(0, MouseSensitivity, 0);
	}
	if (currentY < previousY)
	{
		CamBhoy.Rotate(-MouseSensitivity, 0, 0);
	}
	else if (currentY > previousY)
	{
		CamBhoy.Rotate(MouseSensitivity, 0, 0);
	}
}
bool once = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	if (message == WM_KEYDOWN)
	{
		if (wParam == 'A')
		{
			horizontal = 'l';
		}
		if (wParam == 'D')
		{
			horizontal = 'r'; //Right
		}
		if (wParam == 'W')
		{
			vertical = 'u'; //UP
			forwardtime = true;
		}
		if (wParam == 'S')
		{
			vertical = 'd'; //Down
		}
		if (wParam == 'E')
		{
			if (holding == false)
			{
				holding = true;
			}
			else
			{
				holding = false;
			}
			if (once == true)
			{
				once = false;
			}
		}
	}
	if (message == WM_KEYUP)
	{
		if (wParam == 'A' && horizontal != 'r')
		{
			horizontal = 'n';
		}
		if (wParam == 'D' && horizontal != 'l')
		{
			horizontal = 'n';
		}
		if (wParam == 'W' && vertical != 'd')
		{
			vertical = 'n';
		}
		if (wParam == 'S' && vertical != 'u')
		{
			vertical = 'n';
		}
	}
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSEMOVE:
		//PostQuitMessage(0);
		//ClipCursor(&sizey);
		previousX = currentX;//Replace the old previous
		currentX = MAKEPOINTS(lParam).x;//Collect pointer coords?
		previousY = currentY;
		currentY = MAKEPOINTS(lParam).y;
		MouseMovement();
		break;
		// Note that this tutorial does not handle resizing (WM_SIZE) requests,
		// so we created the window without the resize border.
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

Object test(XMFLOAT3(0, 3, 0));
Object test2(XMFLOAT3(0, 2, 4));
XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
XMMATRIX g_World2;
Object ObjectList[10];
void runthroughtheselads()
{
	ObjectList[0].SetPos(0, 0, 0);
	ObjectList[2].SetPos(2, 0, 0);
	ObjectList[3].SetPos(4, 0, 0);
	ObjectList[4].SetPos(6, 0, 0);
	ObjectList[5].SetPos(8, 0, 0);
	ObjectList[6].SetPos(10, 0, 0);
	ObjectList[7].SetPos(12, 0, 0);
	ObjectList[8].SetPos(14, 0, 0);
	ObjectList[9].SetPos(16, 0, 0);
}

void offsettheseboiis()
{
	float bobly = 0;
	for (int i = 0; i < 10; i++)
	{
		//XMFLOAT3 temp = ObjectList[i].GetPos();
		//temp = XMFLOAT3(temp.x + i, temp.y, temp.z);
		ObjectList[i].Move(i, 0, 0);
		//ObjectList[i].SetPos(i,i,i);
	}
}

float rotty;


XMFLOAT3 temp;
bool legitonce = true;
Object Selector = Object(XMFLOAT3(0, 0, 0), true);
Room bob(5, 5, 5);
ID3D11ShaderResourceView* Animate[4];
int animy = 0;
Target testyy;

ID3D11ShaderResourceView* Goals[10];
wchar_t Dontreusevariablenamesidiot[40] = L"goal01.dds";
void createTexture() //This function will add a number on the end of the suffix you supply it
{
	char *local= new char;
	for (int i = 0; i < 10; i++)
	{
	   itoa(i, local, 10);
	   Dontreusevariablenamesidiot[4] = local[0];
	   CreateDDSTextureFromFile(g_pd3dDevice, Dontreusevariablenamesidiot, nullptr, &Goals[i]);
	   local = new char;
	}
	delete local;
	local = NULL;
}
std::vector<ID3D11ShaderResourceView*> texturelist;
/*void createTexture(short numberoftextures,std::string name) //This function will add a number on the end of the suffix you supply it
{
	const int size = name.size();
	wchar_t** arr = new wchar_t*[size];
	char* local = new char;
	for (int i = 0; i < numberoftextures; i++)
	{
		itoa(i, local, 10);
		Dontreusevariablenamesidiot[4] = local[0];
		CreateDDSTextureFromFile(g_pd3dDevice, Dontreusevariablenamesidiot, nullptr, &Goals[i]);
	}

	for (int i = 0; i < numberoftextures; i++)
	{
		delete[] arr[i];
		arr[i] = NULL;
	}
	delete local;
	local = NULL;
}*/





void createTexture(short numberoftextures)
{
	
	char *end = new char;
	char *large = new char;
	//Used to store up to 100 textures for an object e.g. 0-99. 
	int first=0;
	int second = 0;

	for (int i = 0; i < numberoftextures; i++)
	{
		ID3D11ShaderResourceView* LocalTexture; //Create a texture to append to the list;
		itoa(first, end, 10); //Assign i's number as its ascii equivalanet
		Dontreusevariablenamesidiot[5] = end[0]; //Derefrence the pointer and set the texture name's last number
		CreateDDSTextureFromFile(g_pd3dDevice, Dontreusevariablenamesidiot, nullptr, &LocalTexture); //Create a texture from these details
		texturelist.push_back(LocalTexture); //Push this into the vector that contains this texture
		end = new char;
		large = new char;
		if (first<9)
		{
			first++;
		}
		else
		{
			second++;
			itoa(second, large, 10); //Assign i's number as its ascii equivalanet
			Dontreusevariablenamesidiot[4] = large[0];
			first = 0;
		}
	}
	delete end; //Clear the pointer for use
	end = NULL;
	delete large; //Clear the pointer for use
	large = NULL;
}



struct Scene
{
	//Process the Selector, Map Geometry , Target and Goal are all defined inside this structure and 
	Object *Selector = new Object;
	Target *TargetObject = new Target;
	Room *RoomObject = new Room;

	Scene()
	{
		 *RoomObject=Room(5, 5, 5);
		 *Selector = Object(XMFLOAT3(0, 0, 0), true);
		 Selector->SetTexture(Animate[0]); //Set a texture for the Selector In case I wish to debug it
		 *TargetObject = Target(XMFLOAT3(0,0,0));

	}


	void Process()
	{

	}

};
int animy2 = 0;
XMFLOAT4 vLightDirs[2] =
{
	XMFLOAT4(0, 2, -0, 1.0f), //Ambient light position
	XMFLOAT4(3, 0.0f, -0.3f, 1.0f),  //Pointed Light
};
XMFLOAT4 vLightColors[2] =
{
	XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), //Ambient Light Colour
	XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f) //Directed Light Colour
};
XMMATRIX mLight;
XMMATRIX mLightScale;
XMMATRIX mRotate;
XMVECTOR vLightDir;

void Render()
{
	// Update our time
	if (legitonce == true)
	{
		ObjectList[3].SetPos(XMFLOAT3(6, 7, 0));
		legitonce = false;
		//createTexture(4, "goal1.dds");
		createTexture(29);
	//	CreateDDSTextureFromFile(g_pd3dDevice, L"Test.dds", nullptr, &Animate[0]);
	//	CreateDDSTextureFromFile(g_pd3dDevice, L"Test1.dds", nullptr, &Animate[1]);
	//	CreateDDSTextureFromFile(g_pd3dDevice, L"goal1.dds", nullptr, &Animate[2]);
	//	CreateDDSTextureFromFile(g_pd3dDevice, L"goal1.dds", nullptr, &Animate[3]);
		wchar_t Name[40] = L"Test.dds";
		//Selector.SetTexture(Name, g_pd3dDevice);
		test.SetPos(3, 3, 3);
		//createTexture();
	}
	static float t = 0.0f;
	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;
	}
	if (horizontal == 'l')
	{
		offsettheseboiis();
	}
	if (horizontal == 'r')
	{
		for (int i = 0; i < 10; i++)
		{
			ObjectList[i].Move(XMFLOAT3(0.001f, 0, 0));
		}
	}
	temp = ForwardDirection(CamBhoy.GetRotionFloat3()); //Get the forward direction of the Camera
	if (vertical == 'u')
	{
		CamBhoy.MoveFrom(temp.x*speed*ScaleFactor, -temp.y*speed*ScaleFactor, temp.z*speed*ScaleFactor);
	}
	if (vertical == 'd'& once == false)
	{
		for (int i = 0; i < 10; i++)
		{
			ObjectList[i].Move(i, 0, 0);
		}
		once = true;
	}
	// Modify the color
	//g_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
	//g_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
	//g_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;


	// Setup our lighting parameters
	//XMFLOAT4 vLightDirs[2] =
	///{
	//	XMFLOAT4(0, 2, -0, 1.0f), //Ambient light position
	//	XMFLOAT4(3, 0.0f, -0.3f, 1.0f),  //Pointed Light
	//};
	//XMFLOAT4 vLightColors[2] =
	//{
	//	XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), //Ambient Light Colour
	//	XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f) //Directed Light Colour
	//};

	// Rotate the second light around the origin
	mRotate = XMMatrixRotationY(-2.0f * t);
	vLightDir = XMLoadFloat4(&vLightDirs[1]);
	//XMVECTOR vLightDir = XMLoadFloat3(&(temp*3));
	vLightDir = XMVector3Transform(vLightDir, mRotate);
	XMStoreFloat4(&vLightDirs[1], vLightDir);








	//
	// Clear the back buffer
	//
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);

	//
	// Clear the depth buffer to 1.0 (max depth)
	//
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//
	// Update variables that change once per frame
	//
	Selector.SetPos(CamBhoy.GetPos() + XMFLOAT3(temp.x, -temp.y, temp.z)*SelectorDistance);
	if (holding == true)
	{
		if (Selector.Intersects(ObjectList[3]) == true) //Test for collision between selector cube and target
		{
			once = true;
		}
		else
		{
			holding = false;
		}
	}
	if (once == true)
	{
		ObjectList[3].SetPos(CamBhoy.GetPos() + XMFLOAT3(temp.x, -temp.y, temp.z) * 5);
	}


	testyy.Update();
	XMVECTOR asaw = XMLoadFloat3(&test.GetPos()); //Get the Matrix of the selector cube and feed this into G world
	//test.SetRotation(XMFLOAT3(0, t, 0));
	
	//g_World = XMMatrixRotationY(Selector.GetRotation().y);
	g_World = XMMatrixTranslationFromVector(asaw);
	//g_World *= Selector.GetRotationMatrix();
	CBChangesEveryFrame cb; //Create a constant buffer to be sent to the shader file
	cb.mWorld = XMMatrixTranspose(g_World); //Set the Constant Buffer's world position
//	cb.vMeshColor = g_vMeshColor;  //Set the colour of the object
	cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix()); //Set the view matrix to that of the camera class
	cb.vLightDir[0] = vLightDirs[0];
	cb.vLightDir[1] = vLightDirs[1];
	cb.vLightColor[0] = vLightColors[0];
	cb.vLightColor[1] = vLightColors[1];
	cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	
	
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0); //Start applying rendering settings to be sent to the shader file through the changing constant buffer

	//
	// Render the cube
	//
	//Selector.SetTexture(Animate[animy], g_pd3dDevice);

    Selector.SetTexture(texturelist[animy]);
	ID3D11ShaderResourceView*           tempy = Selector.Texture();
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShaderResources(0, 1, &tempy);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->DrawIndexed(36, 0, 0);



	// Code for rendering the Selector Cube
	if (RenderSelector == true)
	{
		XMVECTOR SelectorMatrix = XMLoadFloat3(&Selector.GetPos()); //Get the Matrix of the selector cube and feed this into G world
		Selector.SetRotation(XMFLOAT3(0, t, 0));
		//g_World = XMMatrixRotationY(Selector.GetRotation().y);
		g_World = XMMatrixTranslationFromVector(SelectorMatrix);
		//g_World *= Selector.GetRotationMatrix();
		CBChangesEveryFrame cb; //Create a constant buffer to be sent to the shader file
		cb.mWorld = XMMatrixTranspose(g_World); //Set the Constant Buffer's world position
		cb.vMeshColor = g_vMeshColor;  //Set the colour of the object
		cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix()); //Set the view matrix to that of the camera class
		cb.vLightDir[0] = vLightDirs[0];
		cb.vLightDir[1] = vLightDirs[1];
		cb.vLightColor[0] = vLightColors[0];
		cb.vLightColor[1] = vLightColors[1];
		cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);


		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0); //Start applying rendering settings to be sent to the shader file through the changing constant buffer

		//
		// Render the cube
		//
		//Selector.SetTexture(Animate[animy]);
		ID3D11ShaderResourceView*           tempy = Selector.Texture();
		g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
		g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
		g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
		g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShaderResources(0, 1, &tempy);
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		cb.vOutputColor = vLightColors[1];
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->DrawIndexed(36, 0, 0);

		// Update the world variable to reflect the current light
		cb.mWorld = XMMatrixTranspose(mLight);
		cb.vOutputColor = vLightColors[1];
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);

		g_pImmediateContext->PSSetShader(g_pPixelShaderSolid, nullptr, 0);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}

	for (unsigned short i = 0; i < 10; i++) //A loop that runs through all objects that need rendering
	{
		XMVECTOR MatrixPosition = XMLoadFloat3(&ObjectList[i].GetPos()); //Get the position of Object[i] and feed this into a Matrix
		g_World = XMMatrixTranslationFromVector(MatrixPosition); //Set the world position for feeding into a constant buffer
		CBChangesEveryFrame cb; //Create a constant buffer
		cb.mWorld = XMMatrixTranspose(g_World); //Set this Position 
		cb.vMeshColor = g_vMeshColor; //Apply the colour as three floats 
		cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix()); //Set the view matrix to the Camera's
		cb.vLightDir[0] = vLightDirs[0];
		cb.vLightDir[1] = vLightDirs[1];
		cb.vLightColor[0] = vLightColors[0];
		cb.vLightColor[1] = vLightColors[1];
		cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);
		
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0); //Start sending the appropriate settings to the shader 
		//
		// Render the cube
		//
		g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
		g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
		g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
		g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
		//g_pImmediateContext->PSSetShaderResources(0, 1, &tempy);
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		g_pImmediateContext->DrawIndexed(36, 0, 0);


	}




	for (int i = 0; i < bob.Floor.size(); i++)
	{
		XMVECTOR MatrixPosition = XMLoadFloat3(&bob.Floor[i]->GetPos()); //Get the position of Object[i] and feed this into a Matrix
		g_World = XMMatrixTranslationFromVector(MatrixPosition); //Set the world position for feeding into a constant buffer
		ID3D11ShaderResourceView*           tempy =bob.Floor[i]->Texture();
		CBChangesEveryFrame cb; //Create a constant buffer
		cb.mWorld = XMMatrixTranspose(g_World); //Set this Position 
		cb.vMeshColor = g_vMeshColor; //Apply the colour as three floats 
		cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix()); //Set the view matrix to the Camera's
		cb.vLightDir[0] = vLightDirs[0];
		cb.vLightDir[1] = vLightDirs[1];
		cb.vLightColor[0] = vLightColors[0];
		cb.vLightColor[1] = vLightColors[1];
		cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0); //Start sending the appropriate settings to the shader 
		//
		// Render the cube
		//
		g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
		g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
		g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
		g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShaderResources(0, 1, &Wood);
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}

	//for (int m = 0; m < 2; m++)
//	{
	//	XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&vLightDirs[m]));
	///	XMMATRIX mLightScale = XMMatrixScaling(1, 1, 1);
	//	mLight = mLightScale * mLight;
		//XMMATRIX temppppp = XMMatrixTranslationFromVector(XMLoadFloat3(&Selector.GetPos()));
		// Update the world variable to reflect the current light
	//	cb.mWorld = XMMatrixTranspose(mLight);
	//	cb.vOutputColor = vLightColors[m];
//		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
		// g_pPixelShaderSolid
	//	g_pImmediateContext->PSSetShader(g_pPixelShaderSolid, nullptr, 0);
	//	g_pImmediateContext->DrawIndexed(36, 0, 0);
//	}

		//vLightDirs

	CBChangesEveryFrame cb1;
	cb1.mWorld = XMMatrixTranspose(g_World);
	cb1.view2 = XMMatrixTranspose(g_View);
	cb1.mProjection2 = XMMatrixTranspose(g_Projection);
	cb1.vLightDir[0] = vLightDirs[0];
	cb1.vLightDir[1] = vLightDirs[1];
	cb1.vLightColor[0] = vLightColors[0];
	cb1.vLightColor[1] = vLightColors[1];
	cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	//g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

	//
	// Render the cube
	//
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->DrawIndexed(36, 0, 0);


	for (int m = 0; m < 2; m++)
	{
		XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&vLightDirs[m]));
		XMMATRIX mLightScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
		mLight = mLightScale * mLight;

		// Update the world variable to reflect the current light
		cb1.mWorld = XMMatrixTranspose(mLight);
		cb1.vOutputColor = vLightColors[m];
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb1, 0, 0);

		g_pImmediateContext->PSSetShader(g_pPixelShaderSolid, nullptr, 0);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}


	if (animy<28)
	{
		animy++;
	}
	else
	{
		animy = 0;
	}
	g_View = CamBhoy.GetViewMatrix();
	g_Projection = CamBhoy.GetProjectionMatrix();
	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, WindowWidth / (FLOAT)WindowHeight, 0.01f, 100.0f);

	CBChangeOnResize yus;
	yus.mProjection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pCBChangeOnResize, 0, nullptr, &yus, 0, 0);
	g_pSwapChain->Present(0, 0);
}
