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
#include "Object.h" //Import the Object class created to store and control all onscreen objects
#include "CameraTest.h" //Import the Camera class that controls the projection and view matrix
#include "VectorMaths.h" //A custom class full of maths functions and XMFLOAT3 Operator Oveloads
#include <vector> //Import the Vector Data Type to hold the textures, its length can dynamically change and is useful for functions that load dynamic amounts of objects
#include "Target.h" //Import the target class that inherets from the Object Class
#include "Goal.h"


#define RenderSelector false //The selector is an invisible cube that is used as a bounding box for collisions with the target block. Setting this to true renders it for debugging purposes
#define SelectorDistance 3.5f //This is the distance that the selector lies from the player camera
#define speed -0.15f //The Speed of camera

#define FPS 60 //The framerate of the sample can be set here
#define targetDelta (1000/FPS) // Attempts at delta timing are made in the sample. Some areas are less reliant however, which is why the framerate is locked to 60
//The sample is undemanding and would struggle to miss this target on any computer that can run DirectX11, as such delta timing was not crucial

#define MouseSensitivity 0.035f //A value that is applied to the appropriate axis by rotating the Camera

//Window Dimensions
#define WindowWidth 800
#define WindowHeight 600

//Delta timing values
float currentDelta = 0.0f;
float ScaleFactor = 0.0f;
float frameEnd = 0.0f;
float timeLast = 0;

//Used to simplify work flow as opposed to DirectX::XMFLOAT3(0,0,0) e.t.c.
using namespace DirectX;


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex //Vertex Buffer. This originally only contained the position and Texture Data, however Normals were added to allow for lighting
{
	XMFLOAT3 Pos; //Shape Data
	XMFLOAT2 Tex; //Texture Layout Data
	XMFLOAT3 Normal; //Normal Faces data used for lighting
};

//Constant Buffer Never Changes is no longer required, originally the view matrix was stored there, but it prevented correct movement.


struct CBChangeOnResize 
{
	XMMATRIX mProjection; //Projection Matrix update on resize of the window
};

struct CBChangesEveryFrame // The main constant buffer object. The world of each object, view, mesh colour and lighting data is handled by this buffer
{
	XMMATRIX mWorld;

	XMMATRIX view2;
	XMMATRIX mProjection2; //Possibly not needed and our view2 matrix;
	//Lighting variables from tutorial 06.
	XMFLOAT4 vMeshColor;
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
ID3D11Buffer*                       g_pCBChangeOnResize = nullptr;
ID3D11Buffer*                       g_pCBChangesEveryFrame = nullptr;
ID3D11ShaderResourceView*           g_pTextureRV = nullptr;
ID3D11SamplerState*                 g_pSamplerLinear = nullptr;
XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
//XMFLOAT4                            g_vMeshColor(0.7f, 0.7f, 0.7f, 1.0f);
XMFLOAT4 g_vMeshColor(0.15f, 0.15f, 0.15f, 1.0f); //Default white light


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
			Render(); //Main render loop is called here
			currentTime = timeGetTime();
			if ((currentTime-timeLast)<(1000/FPS))
			{
				Sleep(currentTime - timeLast);
			}
			timeLast = currentTime;
			frameEnd = timeGetTime();
			currentDelta = (currentTime - frameEnd);
			ScaleFactor = (currentDelta / targetDelta); //Delta Timing is calculated here
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
ID3D11ShaderResourceView* Wood; //Create a memory location for my "wood" texture
ID3D11ShaderResourceView* TargetTex; //Create a memory location for the Target Texture
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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },  //Normal Data is included in the shader layout, included by me for lighting
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
	hr = CompileShaderFromFile(L"Tutorial07.fx", "PSSolid", "ps_4_0", &pPSBlob); //Compile the Solid buffer used for lighting data sent to the shader
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
	SimpleVertex vertices[] = //Vertices, Texture, Normal
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f),XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f),XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) ,XMFLOAT3(0.0f,1.0f,0.0f)},
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) ,XMFLOAT3(0.0f,1.0f,0.0f)},

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) ,XMFLOAT3(0.0f,-1.0f,0.0f)},
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) ,XMFLOAT3(0.0f,-1.0f,0.0f)},
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) ,XMFLOAT3(0.0f,-1.0f,0.0f)},
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) ,XMFLOAT3(0.0f,-1.0f,0.0f)},

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(-1.0f,0,0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f),XMFLOAT3(-1.0f,0,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) ,XMFLOAT3(-1.0f,0,0.0f)},
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) ,XMFLOAT3(-1.0f,0,0.0f)},

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) ,XMFLOAT3(1.0f,0,0.0f)},
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(1.0f,0,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) ,XMFLOAT3(1.0f,0,0.0f)},
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) ,XMFLOAT3(1.0f,0,0.0f)},

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(0,0,-1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) ,XMFLOAT3(0,0,-1.0f)},
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) ,XMFLOAT3(0,0,-1.0f)},
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) ,XMFLOAT3(0,0,-1.0f)},

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f),XMFLOAT3(0,0,1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(0,0,1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f),XMFLOAT3(0,0,1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f),XMFLOAT3(0,0,1.0f) },
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

	bd.ByteWidth = sizeof(CBChangeOnResize);
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangeOnResize);
	if (FAILED(hr))
		return hr;

	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame);
	if (FAILED(hr))
		return hr;

	// Load the Texture
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"seafloor.dds", nullptr, &g_pTextureRV); //Point the texture loaded from file into the desired pointer
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
	CreateDDSTextureFromFile(g_pd3dDevice, L"CustomWood.dds", nullptr, &Wood); //Point the texture loaded from file into the desired pointer
	CreateDDSTextureFromFile(g_pd3dDevice, L"Target.dds", nullptr, &TargetTex); //Point the texture loaded from file into the desired pointer
	// Initialize the world matrices
	g_World = XMMatrixIdentity();

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f); 

	CBChangeOnResize Projection;
	Projection.mProjection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pCBChangeOnResize, 0, nullptr, &Projection, 0, 0);

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
bool Forward; //Used to determine if the player is moving forward or not by assinging this in the input code
bool holding = false; //Determines if the Target block is held or not, and whether it should spin and be set to the selector's position

float previousX; //PreviousX/Y location on screen stored for comparison with CurrentX/Y to detirmine what direction the camera should rotate
float currentX;
float previousY;
float currentY;

CameraTest CamBhoy; //Setting up the camera object for use with a class that will generate the View Matrix and rotation/translation matrix for camera movement


Goal *EndGoal = new Goal;  //Target and Goal Objects Generated for usage. They are rather redundant in their application, but don't cost performance either.
Target *TargetObject = new Target;


//A function for creating a flat Row of set length by storing cube objects in a vector list and returning it.
std::vector<Object*> CreateRow(int length, int row) 
{
	std::vector<Object*> Floor; //Define a Vector list of Object Pointers
	Object *temp = new Object; //Reserve memory for a Object to be passed to the Vector
	for (int i = 0; i < length; i++) //Run through the length and generate the desired amount
	{
		temp = new Object; //Set temp to a new object 
		temp->SetPos(XMFLOAT3(i * 2, 0, row * 2)); //Offset the postion
		temp->SetTexture(Wood); //Set the texture of the object in its own scope
		Floor.push_back(temp); //Send this to the Vector List
	}
	delete temp; //Clear the temp memory location and return the vector List
	temp = NULL;
	return Floor;
}

//Struct object for defining a floor of X length and width. 
struct Room
{
public:
	std::vector<Object*> Floor; //Define a Vector List for storing this floor
	Room(int length, int width, int height) //Unused Height Paramater for future expansion, trouble with building a room was encountered
	{
		for (int i = 0; i < length - 1; i++) //Floor
		{
			std::vector<Object*> AB = CreateRow(width, i); //Create a row in AB
			Floor.insert(Floor.end(), AB.begin(), AB.end()); //Add AB to Floor
		}
	}
	Room()
	{

	}
};


bool grabbing = false; //Boolean for testing if intersection has occured

//Basic Function that moves the mouse depending on the state of the mouse through current and previous positions
void MouseMovement()
{
	if (currentX < previousX) //Moving Left
	{
		CamBhoy.Rotate(0, -MouseSensitivity, 0); //Rotate on the Y axis by Mouse sensitivity
	}
	else if (currentX > previousX) //Right
	{
		CamBhoy.Rotate(0, MouseSensitivity, 0);
	}
	if (currentY < previousY) //Down
	{
		CamBhoy.Rotate(-MouseSensitivity, 0, 0);
	}
	else if (currentY > previousY) //UP
	{
		CamBhoy.Rotate(MouseSensitivity, 0, 0);
	}
}
bool backward = false; //Booleans that control the player's movement
bool left = false;
bool right = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	if (message == WM_KEYDOWN) //If a windows event is equal to a KeyDown message
	{
		if (wParam == 'W') //Forward input for the camera
		{
			Forward = true; //Set the boolean variables for what direction the player is moving in
			backward = false;
		}
		else if (wParam == 'S') //Backward input for the camera
		{
			Forward = false; //Set the boolean variables for what direction the player is moving in
			backward = true;
		}
		if (wParam == 'A') //Left input for the camera
		{
			left = true; //Set the boolean variables for what direction the player is moving in
			right = false;
		}
		else if (wParam == 'D') //Right input for the camera
		{
			left = false; //Set the boolean variables for what direction the player is moving in
			right = true;
		}

		if (wParam == 'E') //Test if the player is grabbing the target block with the "grabbing" boolean
		{
			grabbing = true;
		}
	}
	if (message == WM_KEYUP) //If a windows event is equal to a KeyUp message
	{
		if (wParam=='W') //Set the Forward and backward variables to be false in the case of a player ceasing to move in that direction
		{
			Forward = false;
		}
		else if (wParam == 'S')
		{
			backward = false;
		}
		if (wParam == 'A') //Set the Forward and backward variables to be false in the case of a player ceasing to move in that direction
		{
			left = false;
		}
		else if (wParam == 'D')
		{
			right = false;
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
	case WM_MOUSEMOVE: //On the windows event of mouse movement the appropriate mouse input is processed

		previousX = currentX;//Replace the old previous to be the current X
		currentX = MAKEPOINTS(lParam).x; //Collect the new mouseX coordinates
		previousY = currentY; //Complete the process for the Y axis
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


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------



XMFLOAT3 temp; //Simple Float 3 used for directions
bool Startup = true; //First frame setup variable
Object Selector = Object(XMFLOAT3(0, 0, 0), true); //Selector block used by the player
Room FloorTest(5, 5, 5); //Room struct being used to construct a floor of blocks


int animy = 0; //Integer that controls which frame of animations is being accessed


wchar_t Charholder[40] = L"goal01.dds"; //A charlist that holds the file name that the DDS loader loads from.

std::vector<ID3D11ShaderResourceView*> texturelist; //The Vector list that is defined to hold textures loaded from memory


//Custom function that is used to manipulate the suffix number of the selected texture. 
//Supply the Number of Texture files and ensure texture name is 4 characters long with a theme of 00 on the end e.g. "goal00.dds"
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
		Charholder[5] = end[0]; //Derefrence the pointer and set the texture name's last number
		CreateDDSTextureFromFile(g_pd3dDevice, Charholder, nullptr, &LocalTexture); //Create a texture from these details
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
			Charholder[4] = large[0];
			first = 0;
		}
	}
	delete end; //Clear the pointer for use
	end = NULL;
	delete large; //Clear the pointer for use
	large = NULL;
}




Object ExampleTexturelessBlock;

//Reset/set the Game Settings to the startup
void Reset()
{
	TargetObject->SetPos(11, 5, 7); //Set the position of the target object the player must grab
	TargetObject->SetTexture(TargetTex); //Set the texture of the target to be the correct texture

	EndGoal->function.SetPos(2, 2, 2);  //Do the same for the end goal
	EndGoal->function.SetTexture(texturelist[0]);
	ExampleTexturelessBlock.SetPos(15, 2, 4);
	CamBhoy.SetPos(0, 3, 0); //Set the camera position
}

//Check if the end condtions have been met
void CheckIfWon()
{
	if (EndGoal->function.Intersects(*TargetObject)==true) //The Target has intesected the Goal
	{
		holding = false;
		grabbing = false;
		Reset();
		WndProc(g_hWnd, WM_DESTROY, NULL, NULL); //Shutdown
	}
	else //The attempt failed
	{
		holding = false;
		grabbing = false;
	}
}

void Render()
{
	if (Startup == true)
	{
		Startup = false;
		createTexture(29); //Populate the TextureList
		Reset(); //Set object positions
	}

	// Update our time
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
	temp = ForwardDirection(CamBhoy.GetRotionFloat3()); //Get the forward direction of the Camera
	
	if (Forward == true) //Movement conditions are set by booleans
	{
		CamBhoy.MoveFrom(temp.x*speed*ScaleFactor, -temp.y*speed*ScaleFactor, temp.z*speed*ScaleFactor); //Update the Camera's postion from the previous position by offseting by the temp vector
	}
	else if(backward==true)
	{
		CamBhoy.MoveFrom(temp.x*-speed*ScaleFactor, -temp.y*-speed*ScaleFactor, temp.z*-speed*ScaleFactor); //Move the opposite direction to the forward vector
	}
	if (left == true)
	{
		XMFLOAT3 dir = temp * speed*ScaleFactor;
		CamBhoy.MoveFrom(VectorCrossProduct(dir, XMFLOAT3(0, 1, 0))); //Create a crossproduct for the left direction
	}
	else if (right == true)
	{
		XMFLOAT3 dir = temp * speed*ScaleFactor;
		CamBhoy.MoveFrom(VectorCrossProduct(dir, XMFLOAT3(0, -1, 0))); //Create a crossproduct for the right direciton
	}


	// Setup our lighting parameters
	XMFLOAT4 vLightDirs[2] =
	{
		XMFLOAT4(0, 1, 0, 1.0f),
		XMFLOAT4(1, 0.0f, 0, 1.0f),
	};
	XMFLOAT4 vLightColors[2] =
	{
		XMFLOAT4(1, 0, 0, 1.0f),
		XMFLOAT4(0, 0.0f, 1, 1.0f)
	};



	XMMATRIX mLight;
	XMMATRIX mLightScale;
	XMMATRIX mRotate;
	XMVECTOR vLightDir;

	// Rotate the second light around the origin
	mRotate = XMMatrixRotationY(t);
	vLightDir = XMLoadFloat4(&vLightDirs[1]);
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
	Selector.SetPos(CamBhoy.GetPos() + XMFLOAT3(temp.x, -temp.y, temp.z)*SelectorDistance); //Set the selector to be a set distance Vector from the Camera

	if (grabbing == true&&holding!=true) //Booleans control the logic for whether the target block is being held, a grab is being attempted or if it has been successful
	{
		if (TargetObject->Intersects(Selector) == true) //Test for collision between selector cube and target
		{
			holding = true;
		}
		grabbing = false;
	}
	else if (holding == true &&grabbing==true) //As grabbing is true when a drop is attempted, this means that an intersection test is only made when the player desires, saving cpu cycles.
	{
		CheckIfWon();
	}
	if (holding == true) //If the block is being held, it must be a set distance from the player so that they can see whilst it follows the camera
	{
		TargetObject->SetPos(CamBhoy.GetPos() + XMFLOAT3(temp.x, -temp.y, temp.z) * 5);
	}



	//Rendering of Target Object

	XMVECTOR PositionMatrix = XMLoadFloat3(&TargetObject->GetPos()); //Get the position of the Target Block and store it temporarily

	if (holding == true) //If the block is being held it should not spin.
	{
		g_World = XMMatrixTranslationFromVector(PositionMatrix); //g_world to be the unmodified position as it is not rotating
	}
	else //The object is rotating and the Rotation matrix should be defined and set into g_world and then * by the translation matrix to abide to TRS order matrix multiplication
	{
		TargetObject->SetRotation(XMFLOAT3(t, t, t));
		g_World = XMMatrixRotationRollPitchYaw(TargetObject->GetRotation().x, TargetObject->GetRotation().y, TargetObject->GetRotation().z);
		g_World *= XMMatrixTranslationFromVector(PositionMatrix); //TRS
	}
	CBChangesEveryFrame cb; //Create a constant buffer to be sent to the shader file
	cb.mWorld = XMMatrixTranspose(g_World); //Set the Constant Buffer's world position
	cb.vMeshColor = g_vMeshColor;  //Set the colour of the object
	cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix()); //Set the view matrix to that of the camera class
	cb.vLightDir[0] = vLightDirs[0]; //Define Lighting Data
	cb.vLightDir[1] = vLightDirs[1];
	cb.vLightColor[0] = vLightColors[0];
	cb.vLightColor[1] = vLightColors[1];
	cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);


	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0); //Start applying rendering settings to be sent to the shader file through the changing constant buffer

	//
	// Render the cube
	//
	ID3D11ShaderResourceView*           Tex = TargetObject->Texture(); //Set the Texture to be the Target's stored Texture, storing it in Tex
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0); //Apply Shader Paramaters
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShaderResources(0, 1, &Tex);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	cb.vOutputColor = vLightColors[0];
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->DrawIndexed(36, 0, 0); //Draw the Cube


	//GoalObject Rendering
	PositionMatrix = XMLoadFloat3(&EndGoal->function.GetPos()); //Get the position and store it 
	g_World = XMMatrixTranslationFromVector(PositionMatrix); //Just load the position as the object does not rotate or scale 
	cb; //Create a constant buffer to be sent to the shader file
	cb.mWorld = XMMatrixTranspose(g_World); //Set the Constant Buffer's world position
	cb.vMeshColor = g_vMeshColor;  //Set the colour of the object
	cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix()); //Set the view matrix to that of the camera class
	cb.vLightDir[0] = vLightDirs[0]; //Set Lighting Data
	cb.vLightDir[1] = vLightDirs[1];
	cb.vLightColor[0] = vLightColors[0];
	cb.vLightColor[1] = vLightColors[1];
	cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);


	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0); //Start applying rendering settings to be sent to the shader file through the changing constant buffer

	//
	// Render the cube
	//
	EndGoal->function.SetTexture(texturelist[animy]); //Set the Texture to be one of the "animated" textures found in Texture list. This List only contains 15 frames of unique animation, but items are looped twice to stay for 30fps (In hindsight 4 times would at least have matched the locked framerate of 60)
	Tex = EndGoal->function.Texture();
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0); //Apply Shader Parameters
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShaderResources(0, 1, &Tex);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	cb.vOutputColor = vLightColors[0];
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->DrawIndexed(36, 0, 0); //Draw




	// Code for rendering the Selector Cube if desired. This is the same as above, just the cube is textureless and Selector is never given a texture. This was used for debugging, but can easily be changed with the define setting above
	if (RenderSelector == true)
	{
		XMVECTOR SelectorMatrix = XMLoadFloat3(&Selector.GetPos()); //Get the Matrix of the selector cube and feed this into G world
		Selector.SetRotation(XMFLOAT3(0, t, 0));
		g_World = XMMatrixRotationY(Selector.GetRotation().y);
		g_World *= XMMatrixTranslationFromVector(SelectorMatrix);
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
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
		g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
		g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShaderResources(0, 1, &tempy);
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		cb.vOutputColor = vLightColors[0];
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->DrawIndexed(36, 0, 0);

		// Update the world variable to reflect the current light
		cb.mWorld = XMMatrixTranspose(mLight);
		cb.vOutputColor = vLightColors[1];
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);

		g_pImmediateContext->PSSetShader(g_pPixelShaderSolid, nullptr, 0);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}


	//Object ExampleTexturelessBlock;

	PositionMatrix = XMLoadFloat3(&ExampleTexturelessBlock.GetPos()); //Load the position into the Position Matrix
	g_World= XMMatrixScaling(sinf(-t  / 6),1,tanf(t  / 6)); //Scale the object by t on the X and Y axis. 
	g_World *= XMMatrixRotationRollPitchYaw(-t, -t , t); //Rotate on each axis and multiply g_world by this to abide by TRS
	g_World *= XMMatrixTranslationFromVector(PositionMatrix); //Finally offset by the Translation Matrix, completing TRS order

	//Apply values to the constant buffer
	cb.mWorld = XMMatrixTranspose(g_World); //Set the Constant Buffer's world position
	cb.vMeshColor = g_vMeshColor;  //Set the colour of the object
	cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix()); //Set the view matrix to that of the camera class
	cb.vLightDir[0] = vLightDirs[0]; //Setup Lighting Data
	cb.vLightDir[1] = vLightDirs[1];
	cb.vLightColor[0] = vLightColors[0];
	cb.vLightColor[1] = vLightColors[1];
	cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);


	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0); //Start applying rendering settings to be sent to the shader file through the changing constant buffer
	ID3D11ShaderResourceView*           tempy = Selector.Texture(); //Select Null texture purposefully

	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0); //Apply shader settings
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	cb.vOutputColor = vLightColors[0];
	g_pImmediateContext->PSSetShaderResources(0, 1,&tempy);
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->DrawIndexed(36, 0, 0);


	for (int i = 0; i < FloorTest.Floor.size(); i++) //Draw the floor sequentially
	{
		XMVECTOR MatrixPosition = XMLoadFloat3(&FloorTest.Floor[i]->GetPos()); //Get the position of Object[i] and feed this into a Matrix
		g_World = XMMatrixTranslationFromVector(MatrixPosition); //Set the world position for feeding into a constant buffer
		ID3D11ShaderResourceView*           tempy = FloorTest.Floor[i]->Texture();
		CBChangesEveryFrame cb; //Create a constant buffer
		cb.mWorld = XMMatrixTranspose(g_World); //Set this Position 
		cb.vMeshColor = g_vMeshColor; //Apply the colour as three floats 
		cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix()); //Set the view matrix to the Camera's
		cb.vLightDir[0] = vLightDirs[0]; //Apply Lighting Data
		cb.vLightDir[1] = vLightDirs[1];
		cb.vLightColor[0] = vLightColors[0];
		cb.vLightColor[1] = vLightColors[1];
		cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0); //Start sending the appropriate settings to the shader 
		//
		// Render the cube
		//
		g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0); //Send shader data
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
		g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
		g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
		g_pImmediateContext->PSSetShaderResources(0, 1, &Wood);
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}



	CBChangesEveryFrame cb1;  //Create Constant Buffer for the same pricibles as above
	cb1.mWorld = XMMatrixTranspose(g_World);
	cb1.view2 = XMMatrixTranspose(g_View);
	cb1.mProjection2 = XMMatrixTranspose(CamBhoy.GetProjectionMatrix());
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


	for (int m = 0; m < 2; m++) //Process both lights
	{
		XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&vLightDirs[m])); //Update Light position
		XMMATRIX mLightScale = XMMatrixScaling(0.3f, 0.3f, 0.3f); //Scale the Light
		mLight = mLightScale * mLight; //Apply this to the world of the light

		// Update the world variable to reflect the current light
		cb1.mWorld = XMMatrixTranspose(mLight);
		cb1.vOutputColor = vLightColors[m];
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb1, 0, 0); //Send off lighting data

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
	g_View = CamBhoy.GetViewMatrix(); //Update global view matrix to be that of the camera

	g_pSwapChain->Present(0, 0);
}
