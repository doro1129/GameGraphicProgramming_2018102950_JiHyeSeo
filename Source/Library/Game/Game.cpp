#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    HINSTANCE                       g_hInst = nullptr;
    HWND                            g_hWnd = nullptr;
    D3D_DRIVER_TYPE                 g_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL               g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    ComPtr<ID3D11Device>            g_pD3dDevice = nullptr;
    ComPtr<ID3D11Device1>           g_pD3dDevice1 = nullptr;
    ComPtr<ID3D11DeviceContext>     g_pImmediateContext = nullptr;
    ComPtr<ID3D11DeviceContext1>    g_pImmediateContext1 = nullptr;
    ComPtr<IDXGISwapChain>          g_pDXGISwapChain = nullptr;
    ComPtr<ID3D11RenderTargetView>  g_pRenderTargetView = nullptr;

    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/
    /*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      Function: WindowProc

      Summary:  Defines the behavior of the window—its appearance, how
                it interacts with the user, and so forth

      Args:     HWND hWnd
                  Handle to the window
                UINT uMsg
                  Message code
                WPARAM wParam
                  Additional data that pertains to the message
                LPARAM lParam
                  Additional data that pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    -----------------------------------------------------------------F-F*/
    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    //Declare functions
    HRESULT InitWindow(
        _In_ HINSTANCE hInstance, 
        _In_ INT nCmdShow)
    {
        //Register the window class.
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = PSZ_COURSE_TITLE;
        wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
        
        if (!RegisterClassEx(&wcex))
        {
            return E_FAIL;
        }

        //Create the window.
        g_hInst = hInstance;
        RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        g_hWnd = CreateWindow(
            PSZ_COURSE_TITLE,                                           //Window class
            L"Game Graphics Programming Lab 01: Direct3D 11 Basics",    //Window text
            WS_OVERLAPPEDWINDOW,                                        //Window style

            //Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,

            NULL,                                                       //Parent wiwndow
            NULL,                                                       //Menu
            hInstance,                                                  //Instance handle
            NULL                                                        //Additional application data
        );

        if (g_hWnd == NULL)
        {
            return E_FAIL;
        }

        //Show the window.
        ShowWindow(
            g_hWnd,
            nCmdShow
        );

        return S_OK;
    }

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

        //Create Device/Context
        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            g_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, &g_pD3dDevice, &g_featureLevel, &g_pImmediateContext);

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, &g_pD3dDevice, &g_featureLevel, &g_pImmediateContext);
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        g_pD3dDevice.As(&g_pD3dDevice1);
        g_pImmediateContext.As(&g_pImmediateContext1);

        //Create the Swap Chain
        DXGI_SWAP_CHAIN_DESC desc;
        ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
        desc.Windowed = true;
        desc.BufferDesc.Width = width;
        desc.BufferDesc.Height = height;
        desc.BufferCount = 2;
        desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.OutputWindow = g_hWnd;

        ComPtr<IDXGIDevice> dxgiDevice = nullptr;
        g_pD3dDevice.As(&dxgiDevice);

        ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
        ComPtr<IDXGIAdapter> adapter = nullptr;
       
        hr = dxgiDevice->GetAdapter(&adapter);

        if (SUCCEEDED(hr))
        {
            adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

            hr = dxgiFactory->CreateSwapChain(
                g_pD3dDevice.Get(),
                &desc,
                g_pDXGISwapChain.GetAddressOf()
            );
        }
        
        //Create Render Target
        ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = g_pDXGISwapChain->GetBuffer(
            0,
            IID_PPV_ARGS(&pBackBuffer)
        );

        if (FAILED(hr))
        {
            return hr;
        }

        hr = g_pD3dDevice->CreateRenderTargetView(
            pBackBuffer.Get(),
            nullptr,
            g_pRenderTargetView.GetAddressOf()
        );

        if (FAILED(hr))
        {
            return hr;
        }

        g_pImmediateContext->OMSetRenderTargets(
            1,
            g_pRenderTargetView.GetAddressOf(),
            nullptr
        );

        //Setup the viewport
        D3D11_VIEWPORT vp;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 0.0f;

        g_pImmediateContext->RSSetViewports(1, &vp);

        return hr;
    }

    void CleanupDevice()
    {
        if (g_pRenderTargetView) g_pRenderTargetView.Reset();
        if (g_pDXGISwapChain) g_pDXGISwapChain.Reset();
        if (g_pImmediateContext1) g_pImmediateContext1.Reset();
        if (g_pImmediateContext) g_pImmediateContext.Reset();
        if (g_pD3dDevice1) g_pD3dDevice1.Reset();
        if (g_pD3dDevice) g_pD3dDevice.Reset();
    }

    void Render()
    {
        g_pImmediateContext->ClearRenderTargetView(
            g_pRenderTargetView.Get(),
            Colors::MidnightBlue
        );

        g_pDXGISwapChain->Present(0, 0);
    }

    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
}