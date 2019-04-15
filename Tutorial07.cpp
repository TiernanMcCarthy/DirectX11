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
//const float pi = 3.14159265359f;
//float DegtoRag(float v)
//{
	//float y = v * pi / 180;
	//return y;
//}

/*
XMFLOAT3 VectorSubtraction(XMFLOAT3 v1, XMFLOAT3 v2)
{
	XMFLOAT3 v3 = XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	//XMFLOAT3 v3 = v1 - v2;
	return (v3);
}
XMFLOAT3 EulerToDirection(float x, float y, float z)
{
	XMFLOAT3 temp;
	temp.x = cosf(x) * sinf(y);
	temp.y = sinf(x);
	temp.z = cosf(y) * cosf(x);
	return temp;
}


XMFLOAT3 ForwardDirection(XMFLOAT3 euler)
{
	XMFLOAT3 EulerRotation;
	EulerRotation.x = euler.x;
	EulerRotation.y = euler.y;
	EulerRotation.z = euler.z;
	return EulerToDirection(EulerRotation.x, EulerRotation.y, EulerRotation.z);
}

static XMFLOAT3 operator *(XMFLOAT3 a, float t)
{
	XMFLOAT3 result = XMFLOAT3(a.x*t, a.y*t, a.z*t);
	return result;
}
static XMFLOAT3 operator +(XMFLOAT3 a, XMFLOAT3 t)
{
	XMFLOAT3 result = XMFLOAT3(a.x + t.x, a.y + t.y, a.z + t.y);
	return result;
}

 XMFLOAT3 VectorLerp(XMFLOAT3 A, XMFLOAT3 B, float t)
{
	return A * (1.0f - t) + B * t;
}
 
 /*static bool IntersectingAxis(XMFLOAT3 Axis, AABB Box, XMFLOAT3 StartPoint, XMFLOAT3 EndPoint,  float *Lowest, float *Highest)
 {
	 //Calculate Minimum and Maximum based on the current axis
	 float minimum = 0.0f, maximum = 1.0f;
	 if (Axis == Vector3.right)
	 {
		 minimum = (Box.Left - StartPoint.x) / (EndPoint.x - StartPoint.x);
		 maximum = (Box.Right - StartPoint.x) / (EndPoint.x - StartPoint.x);
	 }
	 else if (Axis == Vector3.up)
	 {
		 minimum = (Box.Bottom - StartPoint.y) / (EndPoint.y - StartPoint.y);
		 maximum = (Box.Top - StartPoint.y) / (EndPoint.y - StartPoint.y);
	 }
	 else if (Axis == Vector3.forward)
	 {
		 minimum = (Box.Back - StartPoint.z) / (EndPoint.z - StartPoint.z);
		 maximum = (Box.Front - StartPoint.z) / (EndPoint.z - StartPoint.z);
	 }

	 if (maximum < minimum)
	 {
		 //Swap values
		 float temp = maximum;
		 maximum = minimum;
		 minimum = temp;
	 }

	 //Eliminate non intersections to optimise

	 if (maximum < Lowest)
		 return false;
	 if (minimum > Highest)
		 return false;

	 Lowest = Mathf.Max(minimum, Lowest);
	 Highest = Mathf.Min(maximum, Highest);

	 if (Lowest > Highest)
		 return false;

	 return true;
 }
/*static bool LineIntersection(AABB Box, XMFLOAT3 StartPoint, XMFLOAT3 EndPoint, OUT Vector3 IntersectionPoint)
{
	//Define initial lowest and highest values
	float Lowest = 0.0f;
	float Highest = 1.0f;

	//Default Value for intersection point
	IntersectionPoint = Vector3.zero;

	//Intersection Checks on every axis 
	if (!IntersectingAxis(Vector3.right, Box, StartPoint, EndPoint, ref Lowest, ref Highest))
		return false;
	if (!IntersectingAxis(Vector3.up, Box, StartPoint, EndPoint, ref Lowest, ref Highest))
		return false;
	if (!IntersectingAxis(Vector3.forward, Box, StartPoint, EndPoint, ref Lowest, ref Highest))
		return false;

	//Calculate Intersection point using interpolation
	//Not the only method
	IntersectionPoint = VectorMaths.VectorLerp(StartPoint, EndPoint, Lowest);
	return true;
}*/




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
XMFLOAT4                            g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }

    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Direct3D 11 Tutorial 7",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if( FAILED(hr) )
    {
        if( pErrorBlob )
        {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
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
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );

        if ( hr == E_INVALIDARG )
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                                    D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        }

        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface( __uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice) );
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory) );
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface( __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2) );
    if ( dxgiFactory2 )
    {
        // DirectX 11.1 or later
        hr = g_pd3dDevice->QueryInterface( __uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1) );
        if (SUCCEEDED(hr))
        {
            (void) g_pImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1) );
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd( g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1 );
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface( __uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain) );
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

        hr = dxgiFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation( g_hWnd, DXGI_MWA_NO_ALT_ENTER );

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &pBackBuffer ) );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
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
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, nullptr, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile( L"Tutorial07.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader );
    if( FAILED( hr ) )
    {    
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
    pVSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile( L"Tutorial07.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader );
    pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

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
    bd.ByteWidth = sizeof( WORD ) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the constant buffers
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBNeverChanges);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangeOnResize);
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBChangeOnResize );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBChangesEveryFrame );
    if( FAILED( hr ) )
        return hr;

    // Load the Texture
    hr = CreateDDSTextureFromFile( g_pd3dDevice, L"seafloor.dds", nullptr, &g_pTextureRV );
    if( FAILED( hr ) )
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
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )
        return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet( 0.0f, 3.0f, -6.0f, 0.0f );
    XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    g_View = XMMatrixLookAtLH( Eye, At, Up );

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose( g_View );
    g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, nullptr, &cbNeverChanges, 0, 0 );

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );
    
    CBChangeOnResize cbChangesOnResize;
    cbChangesOnResize.mProjection = XMMatrixTranspose( g_Projection );
    g_pImmediateContext->UpdateSubresource( g_pCBChangeOnResize, 0, nullptr, &cbChangesOnResize, 0, 0 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pSamplerLinear ) g_pSamplerLinear->Release();
    if( g_pTextureRV ) g_pTextureRV->Release();
    if( g_pCBNeverChanges ) g_pCBNeverChanges->Release();
    if( g_pCBChangeOnResize ) g_pCBChangeOnResize->Release();
    if( g_pCBChangesEveryFrame ) g_pCBChangesEveryFrame->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain1 ) g_pSwapChain1->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext1 ) g_pImmediateContext1->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice1 ) g_pd3dDevice1->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
char horizontal;
char vertical;
bool forwardtime;
float previousX;
float currentX;
float previousY;
float currentY;
bool rotating = true;
CameraTest CamBhoy;
void MouseMovement()
{
	float temp = currentX / previousX;
	//CamBhoy.Rotate(0, 0.00003f*temp, 0);
	if (currentX < previousX)
	{
		CamBhoy.Rotate(0, -currentX * 0.00005f, 0);
	}
	else if (currentX > previousX)
	{
		CamBhoy.Rotate(0, currentX*0.00005f, 0);
	}
	if (currentY < previousY)
	{
		CamBhoy.Rotate(-currentY * 0.00005f, 0, 0);
	}
	else if (currentY > previousY)
	{
		CamBhoy.Rotate(currentY*0.00005f, 0, 0);
	}
	//SetCursorPos(rc.right / 2, rc.bottom / 2);
	//SetPhysicalCursorPos(rc.right / 2, rc.bottom / 2);
	//CamBhoy.SetRotation()
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
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
    switch( message )
    {
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;
	case WM_MOUSEMOVE:
		//PostQuitMessage(0);
		MouseMovement();
		//ClipCursor(&sizey);
		previousX = currentX;//Replace the old previous
		currentX = MAKEPOINTS(lParam).x;//Collect pointer coords?
		previousY = currentY;
		currentY = MAKEPOINTS(lParam).y;
		break;
        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.
    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}

Object test(XMFLOAT3(0,3,0));
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


	//for (int i = 0; i < 10; i++)
	//{
		//XMFLOAT3 temp = ObjectList[i].GetPos();
		//ObjectList[i].SetPos(i,i,i);
	//}//
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
bool once = false;
float speed = 0.0003f;
XMFLOAT3 temp;
Object Selector = Object(XMFLOAT3(0, 0, 0), true);
void Render()
{
    // Update our time
	ObjectList[3].SetPos(XMFLOAT3(6, 7, 0));
    static float t = 0.0f;
    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static ULONGLONG timeStart = 0;
        ULONGLONG timeCur = GetTickCount64();
        if( timeStart == 0 )
            timeStart = timeCur;
        t = ( timeCur - timeStart ) / 1000.0f;
    }
	if (horizontal=='l')
	{
		offsettheseboiis();
	}
	if (horizontal == 'r')
	{
		//blob.x += 0.0003f;
		//CamBhoy.SetRotation(0, rotty, 0);
		for (int i = 0; i < 10; i++)
		{
			//XMFLOAT3 temp = ObjectList[i].GetPos();
			//temp = XMFLOAT3(temp.x + i, temp.y, temp.z);
			ObjectList[i].Move(XMFLOAT3(0.001f, 0, 0));
			//ObjectList[i].SetPos(i,i,i);
		}
		//	Eye = XMVectorSet(blob.x, 0, -5.0f, 0.0f);
	}
	temp = ForwardDirection(CamBhoy.GetRotionFloat3()); //There's something wrong with this Idk why
	if (vertical == 'u')
	{
	
		//Eye = XMVectorSet(0.0f, -15, -6.0f, 0.0f);
		CamBhoy.MoveFrom(temp.x*speed, -temp.y*speed , temp.z*speed);
		//CamBhoy.GetPos();
	}
	if (vertical == 'd'& once==false)
	{
		//runthroughtheselads();
		for (int i = 0; i < 10; i++)
		{
			//XMFLOAT3 temp = ObjectList[i].GetPos();
			//temp = XMFLOAT3(temp.x + i, temp.y, temp.z);
			ObjectList[i].Move(i, 0, 0);
			//ObjectList[i].SetPos(i,i,i);
		}
		once = true;
	}
	//CamBhoy.SetPos(0, 3, 0);


    // Rotate cube around the origin
	//XMVECTOR ech = XMLoadFloat3(&test.GetPos());
	//XMVECTOR ech2 = XMLoadFloat3(&test2.GetPos());
	//g_World = XMMatrixTranslationFromVector(ech);
	//g_World2 = XMMatrixTranslationFromVector(ech2);
	//g_World *= XMMatrixRotationY(t);
	//	XMMatrixRotationY( t );
		//XMVector3Transform(ech, );
    // Modify the color
    g_vMeshColor.x = ( sinf( t * 1.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.y = ( cosf( t * 3.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.z = ( sinf( t * 5.0f ) + 1.0f ) * 0.5f;

    //
    // Clear the back buffer
    //
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, Colors::MidnightBlue );

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    //
    // Update variables that change once per frame
    //
	AABB bob =  AABB( XMFLOAT3(0, 0, 0),  XMFLOAT3(1, 1, 1));
	AABB bob2 =  AABB( XMFLOAT3(0, 0, 0),  XMFLOAT3(1, 1, 1));
	if (Intersects(bob, bob2) == true)
	{
		once = false;
	}
	//ObjectList[3].box.pos = ObjectList[3].GetPos();
	//Selector.box.pos = Selector.GetPos();
	//if (Intersects(ObjectList[3].box,Selector.box) == true)
	//{
	//	once = false;
	//}

	//Loop of rendering multiple objects
	/*for (unsigned short i = 0; i < 10; i++)
	{
		XMVECTOR MatrixPosition = XMLoadFloat3(&ObjectList[i].GetPos());
		g_World = XMMatrixTranslationFromVector(MatrixPosition);
		CBChangesEveryFrame cb;
		cb.mWorld = XMMatrixTranspose(g_World);
		cb.vMeshColor = g_vMeshColor;
		cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix());
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
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
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}*/
	//ObjectList[1].SetPos(3, 0, 3);
	//Selector.SetPos(CamBhoy.GetPos()+temp*4);
	Selector.SetPos(CamBhoy.GetPos() + XMFLOAT3(temp.x,-temp.y,temp.z)*8);


	XMVECTOR ShittyMatrix = XMLoadFloat3(&Selector.GetPos());
	g_World = XMMatrixTranslationFromVector(ShittyMatrix);
	CBChangesEveryFrame cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.vMeshColor = g_vMeshColor;
	cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix());
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);

	//Define a start and end point
	//XMFLOAT3 LineStart = temp;
	//XMFLOAT3 LineEnd = LineIntersections.GetPos();


	//Perform the intersection
	//XMFLOAT3 *i = new XMFLOAT3;
	//if (LineIntersections.box.LineIntersection(LineIntersections.box, LineStart, LineEnd, i)==true)
	//{
	//	vertical='d';
//	}

//	i = NULL;
//	delete i;
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
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->DrawIndexed(36, 0, 0);

	for (unsigned short i = 0; i < 10; i++)
	{
		//ObjectList[i].SetPos(ObjectList[i].GetPos());
		XMVECTOR MatrixPosition = XMLoadFloat3(&ObjectList[i].GetPos());
		g_World = XMMatrixTranslationFromVector(MatrixPosition);
		CBChangesEveryFrame cb;
		cb.mWorld = XMMatrixTranspose(g_World);
		cb.vMeshColor = g_vMeshColor;
		cb.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix());
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
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
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}

	/*// Update variables that change once per frame
	//
	CBChangesEveryFrame cb2;
	cb2.mWorld = XMMatrixTranspose(g_World2);
	cb2.vMeshColor = g_vMeshColor;
	cb2.view2 = XMMatrixTranspose(CamBhoy.GetViewMatrix());
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cb2, 0, 0);

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
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->DrawIndexed(36, 0, 0);
    //
    // Present our back buffer to our front buffer*/
    //
	//CamBhoy.UpdateViewMatrix();
	g_View = CamBhoy.GetViewMatrix();
    g_pSwapChain->Present( 0, 0 );
}
