#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                 m_immediateContext, m_immediateContext1, m_swapChain,
                 m_swapChain1, m_renderTargetView, m_depthStencil,
                 m_depthStencilView, m_cbChangeOnResize, m_camera,
                 m_projection, m_renderables, m_vertexShaders, 
                 m_pixelShaders].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL),
        m_featureLevel(D3D_FEATURE_LEVEL_11_0),
        m_d3dDevice(nullptr),
        m_d3dDevice1(nullptr),
        m_immediateContext(nullptr),
        m_immediateContext1(nullptr),
        m_swapChain(nullptr),
        m_swapChain1(nullptr),
        m_renderTargetView(nullptr),
        m_depthStencil(nullptr),
        m_depthStencilView(nullptr),
        m_cbChangeOnResize(nullptr),
        m_cbLights(nullptr),
        m_pszMainSceneName(nullptr),
        m_camera(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f)),
        m_projection(XMMATRIX()),
        m_renderables(std::unordered_map<std::wstring, std::shared_ptr<Renderable>>()),
        m_aPointLights(),
        m_vertexShaders(std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>()),
        m_pixelShaders(std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>()),
        m_scenes(std::unordered_map<std::wstring, std::shared_ptr<Scene>>())
    { }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_cbChangeOnResize, 
                 m_projection, m_cbLights, m_camera, m_vertexShaders, 
                 m_pixelShaders, m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        HRESULT hr = S_OK;

        DWORD createDeviceFlags = 0;
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
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        m_d3dDevice.As(&m_d3dDevice1);
        m_immediateContext.As(&m_immediateContext1);

        //Create the Swap Chain
        DXGI_SWAP_CHAIN_DESC desc =
        {
            .BufferDesc = {.Width = width,
                            .Height = height,
                            .RefreshRate = {.Numerator = 60,
                                             .Denominator = 1},
                            .Format = DXGI_FORMAT_R8G8B8A8_UNORM},
            .SampleDesc = {.Count = 1,
                           .Quality = 0},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .OutputWindow = hWnd,
            .Windowed = true
        };

        ComPtr<IDXGIDevice> dxgiDevice = nullptr;
        m_d3dDevice.As(&dxgiDevice);

        ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
        ComPtr<IDXGIAdapter> adapter = nullptr;

        hr = dxgiDevice->GetAdapter(&adapter);

        if (SUCCEEDED(hr))
        {
            adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

            hr = dxgiFactory->CreateSwapChain(
                m_d3dDevice.Get(),
                &desc,
                m_swapChain.GetAddressOf()
            );
        }
        if (FAILED(hr))
        {
            return hr;
        }

        //Create Render Target
        ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = m_swapChain->GetBuffer(
            0,
            IID_PPV_ARGS(pBackBuffer.GetAddressOf())
        );
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(
            pBackBuffer.Get(),
            nullptr,
            m_renderTargetView.GetAddressOf()
        );
        if (FAILED(hr))
        {
            return hr;
        }

        //Create depth stencil texture an the depth stencil view
        ComPtr<ID3D11Texture2D> pDepthStencil = nullptr;
        D3D11_TEXTURE2D_DESC descDepth = {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1,
                            .Quality = 0 },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0
        };

        hr = m_d3dDevice->CreateTexture2D(
            &descDepth,
            nullptr,
            m_depthStencil.GetAddressOf()
        );
        if (FAILED(hr))
        {
            return hr;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };

        hr = m_d3dDevice->CreateDepthStencilView(
            m_depthStencil.Get(),
            &descDSV,
            m_depthStencilView.GetAddressOf()
        );
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(
            1,
            m_renderTargetView.GetAddressOf(),
            m_depthStencilView.Get()
        );

        //Setup the viewport
        D3D11_VIEWPORT vp = {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)800,
            .Height = (FLOAT)600,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };
        m_immediateContext->RSSetViewports(1, &vp);

        //Initialize view matrix and the projection matrix
        m_camera.Initialize(m_d3dDevice.Get());

        m_projection = XMMatrixPerspectiveFovLH(
            XM_PIDIV2,
            static_cast<FLOAT>(width) / static_cast<FLOAT>(height),
            0.01f,
            100.0f
        );

        //Initialize the shaders, then the renderables
        for (auto iter = m_renderables.begin(); iter != m_renderables.end(); iter++)
        {
            hr = iter->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto iter = m_vertexShaders.begin(); iter != m_vertexShaders.end(); iter++)
        {
            hr = iter->second->Initialize(m_d3dDevice.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto iter = m_pixelShaders.begin(); iter != m_pixelShaders.end(); iter++)
        {
            hr = iter->second->Initialize(m_d3dDevice.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto iter = m_scenes.begin(); iter != m_scenes.end(); iter++)
        {
            hr = iter->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        //Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        //Create the CBChangeOnResize constant buffer and Set
        D3D11_BUFFER_DESC cbChangeOnResizebd = {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
            .StructureByteStride = 0
        };
        D3D11_SUBRESOURCE_DATA initData = {
            .pSysMem = &cbChangeOnResizebd,
            .SysMemPitch = 0,
            .SysMemSlicePitch = 0
        };
        hr = m_d3dDevice->CreateBuffer(
            &cbChangeOnResizebd,
            &initData,
            m_cbChangeOnResize.GetAddressOf()
        );
        if (FAILED(hr))
        {
            return hr;
        }
        CBChangeOnResize cb = {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(
            m_cbChangeOnResize.Get(),
            0u,
            nullptr,
            &cb,
            0u,
            0u
        );

        //Create the CBLights constant buffer 
        D3D11_BUFFER_DESC cbLightsbd = {
            .ByteWidth = sizeof(CBLights),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
            .StructureByteStride = 0
        };
        initData = {
            .pSysMem = &cbLightsbd,
            .SysMemPitch = 0,
            .SysMemSlicePitch = 0
        };
        hr = m_d3dDevice->CreateBuffer(
            &cbLightsbd,
            &initData,
            m_cbLights.GetAddressOf()
        );
        if (FAILED(hr))
        {
            return hr;
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable

      Summary:  Add a renderable object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Shared pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
    {
        // checks if the key already exists in the renderable hash map
        if (m_renderables.count(pszRenderableName) > 0)
        {
            // key already exists
            return E_FAIL;
        }

        // add the renderable
        m_renderables.insert(std::make_pair(pszRenderableName, renderable));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPointLight

      Summary:  Add a point light

      Args:     size_t index
                  Index of the point light
                const std::shared_ptr<PointLight>& pointLight
                  Shared pointer to the point light object

      Modifies: [m_aPointLights].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
    {
        if (index >= NUM_LIGHTS)
        {
            return E_FAIL;
        }

        m_aPointLights[index] = pPointLight;
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader

      Summary:  Add the vertex shader into the renderer

      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add

      Modifies: [m_vertexShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        // checks if the key already exists in the renderable hash map
        if (m_vertexShaders.count(pszVertexShaderName) > 0)
        {
            // key already exists
            return E_FAIL;
        }

        // add the vertex shader
        m_vertexShaders.insert(std::make_pair(pszVertexShaderName, vertexShader));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader

      Summary:  Add the pixel shader into the renderer

      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add

      Modifies: [m_pixelShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        // checks if the key already exists in the renderable hash map
        if (m_pixelShaders.count(pszPixelShaderName) > 0)
        {
            // key already exists
            return E_FAIL;
        }

        // add the pixel shader
        m_pixelShaders.insert(std::make_pair(pszPixelShaderName, pixelShader));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add a scene

      Args:     PCWSTR pszSceneName
                  Key of a scene
                const std::filesystem::path& sceneFilePath
                  File path to initialize a scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath)
    {
        // checks if the key already exists in the renderable hash map
        if (m_scenes.count(pszSceneName) > 0)
        {
            // key already exists
            return E_FAIL;
        }
        
        // add the pixel shader
        m_scenes.insert(std::make_pair(pszSceneName, std::make_shared<Scene>(sceneFilePath)));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  Name of the scene to set as the main scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (m_pszMainSceneName == pszSceneName)
        {
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Add the pixel shader into the renderer and initialize it

      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data

      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(
        _In_ const DirectionsInput& directions,
        _In_ const MouseRelativeMovement& mouseRelativeMovement,
        _In_ FLOAT deltaTime
    )
    {
        m_camera.HandleInput(
            directions,
            mouseRelativeMovement,
            deltaTime
        );
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        //Update the renderer - Update all renderable objects
        for (auto iter = m_renderables.begin(); iter != m_renderables.end(); iter++)
        {
            iter->second->Update(deltaTime);
        }
        m_camera.Update(deltaTime);
        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            m_aPointLights[i]->Update(deltaTime);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        //Clear the back buffer
        m_immediateContext->ClearRenderTargetView(
            m_renderTargetView.Get(),
            Colors::MidnightBlue
        );

        //Clear the depth buffer to 1.0 (maximum depth)
        m_immediateContext->ClearDepthStencilView(
            m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH,
            1.0f,
            0
        );

        //Create camera constant buffer and update
        XMFLOAT4 cameraPosition = XMFLOAT4();
        XMStoreFloat4(&cameraPosition, m_camera.GetEye());
        CBChangeOnCameraMovement cb = {
            .View = XMMatrixTranspose(m_camera.GetView()),
            .CameraPosition = cameraPosition
        };
        m_immediateContext->UpdateSubresource(
            m_camera.GetConstantBuffer().Get(),
            0u,
            nullptr,
            &cb,
            0u,
            0u
        );

        //Update the lights constant buffer
        CBLights cb1 = {};
        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            cb1.LightPositions[i] = m_aPointLights[i]->GetPosition();
            cb1.LightColors[i] = m_aPointLights[i]->GetColor();
        }
        m_immediateContext->UpdateSubresource(
            m_cbLights.Get(),
            0u,
            nullptr,
            &cb1,
            0u,
            0u
        );

        //For each renderables
        UINT uStride = sizeof(SimpleVertex);
        UINT uOffset = 0;
        for (auto iter = m_renderables.begin(); iter != m_renderables.end(); iter++)
        {
            std::shared_ptr<Renderable> renderable = iter->second;

            //Set the vertex buffer, index buffer, and the input layout
            m_immediateContext->IASetVertexBuffers(
                0u,
                1u,
                renderable->GetVertexBuffer().GetAddressOf(),
                &uStride,
                &uOffset
            );
            m_immediateContext->IASetIndexBuffer(
                renderable->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );
            m_immediateContext->IASetInputLayout(renderable->GetVertexLayout().Get());

            //Create renderable constant buffer and update
            CBChangesEveryFrame cb2 = {
            .World = XMMatrixTranspose(renderable->GetWorldMatrix()),
            .OutputColor = renderable->GetOutputColor()
            };
            m_immediateContext->UpdateSubresource(
                renderable->GetConstantBuffer().Get(),
                0u,
                nullptr,
                &cb2,
                0u,
                0u
            );

            //Set Shaders and Constant Buffers, Shader Resources, and Samplers
            m_immediateContext->VSSetShader(
                renderable->GetVertexShader().Get(),
                nullptr,
                0
            );
            m_immediateContext->VSSetConstantBuffers(
                0,
                1,
                m_camera.GetConstantBuffer().GetAddressOf()
            );
            m_immediateContext->VSSetConstantBuffers(
                1,
                1,
                m_cbChangeOnResize.GetAddressOf()
            );
            m_immediateContext->VSSetConstantBuffers(
                2,
                1,
                renderable->GetConstantBuffer().GetAddressOf()
            );
            m_immediateContext->PSSetConstantBuffers(
                0,
                1,
                m_camera.GetConstantBuffer().GetAddressOf()
            );
            m_immediateContext->PSSetConstantBuffers(
                2,
                1,
                renderable->GetConstantBuffer().GetAddressOf()
            );
            m_immediateContext->PSSetConstantBuffers(
                3,
                1,
                m_cbLights.GetAddressOf()
            );
            m_immediateContext->PSSetShader(
                renderable->GetPixelShader().Get(),
                nullptr,
                0
            );
            if (renderable->HasTexture())
            {
                for (UINT i = 0; i < renderable->GetNumMeshes(); ++i)
                {
                    UINT materialIndex = renderable->GetMesh(i).uMaterialIndex;
                    assert(materialIndex < renderable->GetNumMaterials());
                    std::shared_ptr<Texture> diffuse = renderable->GetMaterial(materialIndex).pDiffuse;
                    m_immediateContext->PSSetShaderResources(
                        0,
                        1,
                        diffuse->GetTextureResourceView().GetAddressOf()
                    );
                    m_immediateContext->PSSetSamplers(
                        0,
                        1,
                        diffuse->GetSamplerState().GetAddressOf()
                    );

                    //Draw with texture
                    m_immediateContext->DrawIndexed(
                        renderable->GetMesh(i).uNumIndices,
                        renderable->GetMesh(i).uBaseVertex,
                        renderable->GetMesh(i).uBaseIndex
                    );
                }
            }
            else
            {
                //Draw without texture
                m_immediateContext->DrawIndexed(renderable->GetNumIndices(), 0, 0);
            }
        }
        //Render voxels of the main scene
        UINT uStrides[2] = { sizeof(SimpleVertex), sizeof(InstanceData) };
        UINT uOffsets[2] = { 0, 0 };
        for (auto iter = m_scenes.begin(); iter != m_scenes.end(); iter++)
        {
            std::vector<std::shared_ptr<Voxel>> voxel = iter->second->GetVoxels();
            for (UINT i = 0; i < voxel.size(); i++)
            {
                ComPtr<ID3D11Buffer> VoxelBuffers[2] = { voxel[i]->GetVertexBuffer(), voxel[i]->GetInstanceBuffer() };

                //Set the buffers, and input layout
                m_immediateContext->IASetVertexBuffers(
                    0u,
                    2u,
                    VoxelBuffers->GetAddressOf(),
                    uStrides,
                    uOffsets
                );
                m_immediateContext->IASetIndexBuffer(
                    voxel[i]->GetIndexBuffer().Get(),
                    DXGI_FORMAT_R16_UINT,
                    0
                );
                m_immediateContext->IASetInputLayout(voxel[i]->GetVertexLayout().Get());

                //Update the constant buffers
                CBChangesEveryFrame cb2 = {
                .World = XMMatrixTranspose(voxel[i]->GetWorldMatrix()),
                .OutputColor = voxel[i]->GetOutputColor()
                };
                m_immediateContext->UpdateSubresource(
                    voxel[i]->GetConstantBuffer().Get(),
                    0u,
                    nullptr,
                    &cb2,
                    0u,
                    0u
                );

                //Set the shaders and their input
                m_immediateContext->VSSetShader(
                    voxel[i]->GetVertexShader().Get(),
                    nullptr,
                    0
                );
                m_immediateContext->VSSetConstantBuffers(
                    0,
                    1,
                    m_camera.GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->VSSetConstantBuffers(
                    1,
                    1,
                    m_cbChangeOnResize.GetAddressOf()
                );
                m_immediateContext->VSSetConstantBuffers(
                    2,
                    1,
                    voxel[i]->GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    0,
                    1,
                    m_camera.GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    2,
                    1,
                    voxel[i]->GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    3,
                    1,
                    m_cbLights.GetAddressOf()
                );
                m_immediateContext->PSSetShader(
                    voxel[i]->GetPixelShader().Get(),
                    nullptr,
                    0
                );

                m_immediateContext->DrawIndexedInstanced(voxel[i]->GetNumIndices(), voxel[i]->GetNumInstances(), 0, 0, 0);
            }
        }
        //Present
        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable

      Summary:  Sets the vertex shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::const_iterator iRenderable = m_renderables.find(pszRenderableName);
        std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>::const_iterator iVertexShader = m_vertexShaders.find(pszVertexShaderName);

        if (iRenderable == m_renderables.end() || iVertexShader == m_vertexShaders.end())
        {
            return E_FAIL;
        }

        iRenderable->second->SetVertexShader(iVertexShader->second);

        return S_OK;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable

      Summary:  Sets the pixel shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
    {
        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::const_iterator iRenderable = m_renderables.find(pszRenderableName);
        std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>::const_iterator iPixelShader = m_pixelShaders.find(pszPixelShaderName);

        if (iRenderable == m_renderables.end() || iPixelShader == m_pixelShaders.end())
        {
            return E_FAIL;
        }

        iRenderable->second->SetPixelShader(iPixelShader->second);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfScene

      Summary:  Sets the vertex shader for the voxels in a scene

      Args:     PCWSTR pszSceneName
                  Key of the scene
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
    {
        std::unordered_map<std::wstring, std::shared_ptr<Scene>>::const_iterator iScene = m_scenes.find(pszSceneName);
        std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>::const_iterator iVertexShader = m_vertexShaders.find(pszVertexShaderName);

        if (iScene == m_scenes.end() || iVertexShader == m_vertexShaders.end())
        {
            return E_FAIL;
        }

        std::vector<std::shared_ptr<Voxel>> voxel = iScene->second->GetVoxels();
        for (UINT i = 0; i < voxel.size(); i++)
        {
            voxel[i]->SetVertexShader(iVertexShader->second);
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfScene

      Summary:  Sets the pixel shader for the voxels in a scene

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
    {
        std::unordered_map<std::wstring, std::shared_ptr<Scene>>::const_iterator iScene = m_scenes.find(pszSceneName);
        std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>::const_iterator iPixelShader = m_pixelShaders.find(pszPixelShaderName);

        if (iScene == m_scenes.end() || iPixelShader == m_pixelShaders.end())
        {
            return E_FAIL;
        }

        std::vector<std::shared_ptr<Voxel>> voxel = iScene->second->GetVoxels();
        for (UINT i = 0; i < voxel.size(); i++)
        {
            voxel[i]->SetPixelShader(iPixelShader->second);
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}