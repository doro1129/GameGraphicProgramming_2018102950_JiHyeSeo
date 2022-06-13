#include "Renderer/Renderer.h"

namespace library
{

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer
      Summary:  Constructor
      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_depthStencil,
                  m_depthStencilView, m_cbChangeOnResize, m_cbShadowMatrix,
                  m_pszMainSceneName, m_camera, m_projection, m_scenes
                  m_invalidTexture, m_shadowMapTexture, m_shadowVertexShader,
                  m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice()
        , m_d3dDevice1()
        , m_immediateContext()
        , m_immediateContext1()
        , m_swapChain()
        , m_swapChain1()
        , m_renderTargetView()
        , m_depthStencil()
        , m_depthStencilView()
        , m_cbChangeOnResize()
        , m_cbLights()
        , m_cbShadowMatrix()
        , m_pszMainSceneName(nullptr)
        , m_padding{ '\0' }
        , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_projection()
        , m_scenes()
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
        , m_shadowMapTexture()
        , m_shadowVertexShader()
        , m_shadowPixelShader()
    { }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize
      Summary:  Creates Direct3D device and swap chain
      Args:     HWND hWnd
                  Handle to the window
      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView, m_vertexShader,
                  m_vertexLayout, m_pixelShader, m_vertexBuffer
                  m_cbShadowMatrix].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (SUCCEEDED(hr))
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u, .Quality = 0u },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        D3D11_BUFFER_DESC cbShadowMatrix =
        {
            .ByteWidth = sizeof(CBShadowMatrix),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&cbShadowMatrix, nullptr, m_cbShadowMatrix.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_camera.Initialize(m_d3dDevice.Get());

        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene
      Summary:  Add scene to renderer
      Args:     PCWSTR pszSceneName
                  The name of the scene
                const std::shared_ptr<Scene>&
                  The shared pointer to Scene
      Modifies: [m_scenes].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull
      Summary:  Return scene with the given name or null
      Args:     PCWSTR pszSceneName
                  The name of the scene
      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene
      Summary:  Set the main scene
      Args:     PCWSTR pszSceneName
                  The name of the scene
      Modifies: [m_pszMainSceneName].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetShadowMapShaders
      Summary:  Set shaders for the shadow mapping
      Args:     std::shared_ptr<ShadowVertexShader>
                  vertex shader
                std::shared_ptr<PixelShader>
                  pixel shader
      Modifies: [m_shadowVertexShader, m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::SetShadowMapShaders(_In_ std::shared_ptr<ShadowVertexShader> vertexShader, _In_ std::shared_ptr<PixelShader> pixelShader)
    {
        m_shadowVertexShader = move(vertexShader);
        m_shadowPixelShader = move(pixelShader);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput
      Summary:  Handle user mouse input
      Args:     DirectionsInput& directions
                MouseRelativeMovement& mouseRelativeMovement
                FLOAT deltaTime
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update
      Summary:  Update the renderables each frame
      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_camera.Update(deltaTime);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render
      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        //RenderSceneToTexture();

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

        for (auto iScene = m_scenes.begin(); iScene != m_scenes.end(); iScene++)
        {
            //Update the lights constant buffer
            CBLights cb1 = {};
            for (int i = 0; i < NUM_LIGHTS; i++)
            {
                FLOAT attenuationDistance = iScene->second->GetPointLight(i)->GetAttenuationDistance();
                FLOAT attenuationDistanceSquared = attenuationDistance * attenuationDistance;
                cb1.PointLights[i] = {
                    .LightPositions = iScene->second->GetPointLight(i)->GetPosition(),
                    .LightColors = iScene->second->GetPointLight(i)->GetColor(),
                    .AttenuationDistance = XMFLOAT4(
                        attenuationDistance,
                        attenuationDistance,
                        attenuationDistanceSquared,
                        attenuationDistanceSquared
                    )
                };
            }
            m_immediateContext->UpdateSubresource(
                m_cbLights.Get(),
                0u,
                nullptr,
                &cb1,
                0u,
                0u
            );

            //For each renderables (provided by main scene)
            for (auto iRenderable = iScene->second->GetRenderables().begin(); iRenderable != iScene->second->GetRenderables().end(); iRenderable++)
            {
                UINT uStrides[2] = { sizeof(SimpleVertex), sizeof(NormalData) };
                UINT uOffsets[2] = { 0u, 0u };
                ComPtr<ID3D11Buffer> aBuffers[2] =
                {
                    iRenderable->second->GetVertexBuffer(),
                    iRenderable->second->GetNormalBuffer()
                };
                //Set the vertex buffer, index buffer, and the input layout
                m_immediateContext->IASetVertexBuffers(
                    0u,
                    2u,
                    aBuffers->GetAddressOf(),
                    uStrides,
                    uOffsets
                );
                m_immediateContext->IASetIndexBuffer(
                    iRenderable->second->GetIndexBuffer().Get(),
                    DXGI_FORMAT_R16_UINT,
                    0
                );
                m_immediateContext->IASetInputLayout(iRenderable->second->GetVertexLayout().Get());

                //Create renderable constant buffer and update
                CBChangesEveryFrame cbChangesEveryFrame = {
                    .World = XMMatrixTranspose(iRenderable->second->GetWorldMatrix()),
                    .OutputColor = iRenderable->second->GetOutputColor(),
                    .HasNormalMap = iRenderable->second->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(
                    iRenderable->second->GetConstantBuffer().Get(),
                    0u,
                    nullptr,
                    &cbChangesEveryFrame,
                    0u,
                    0u
                );

                //Set Shaders and Constant Buffers, Shader Resources, and Samplers
                m_immediateContext->VSSetShader(
                    iRenderable->second->GetVertexShader().Get(),
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
                    iRenderable->second->GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->VSSetConstantBuffers(
                    3,
                    1,
                    m_cbLights.GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    0,
                    1,
                    m_camera.GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    2,
                    1,
                    iRenderable->second->GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    3,
                    1,
                    m_cbLights.GetAddressOf()
                );
                m_immediateContext->PSSetShader(
                    iRenderable->second->GetPixelShader().Get(),
                    nullptr,
                    0
                );

                if (iScene->second->GetSkyBox())
                {
                    eTextureSamplerType textureSamplerType = iScene->second->GetSkyBox()->GetSkyboxTexture()->GetSamplerType();
                    m_immediateContext->PSSetShaderResources(
                        2,
                        1,
                        iScene->second->GetSkyBox()->GetSkyboxTexture()->GetTextureResourceView().GetAddressOf()
                    );
                    m_immediateContext->PSSetSamplers(
                        2,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                    );
                }

                if (iRenderable->second->HasTexture())
                {
                    for (UINT i = 0; i < iRenderable->second->GetNumMeshes(); ++i)
                    {
                        UINT materialIndex = iRenderable->second->GetMesh(i).uMaterialIndex;
                        assert(materialIndex < iRenderable->second->GetNumMaterials());

                        eTextureSamplerType textureSamplerType = iRenderable->second->GetMaterial(materialIndex)->pDiffuse->GetSamplerType();
                        m_immediateContext->PSSetShaderResources(
                            0,
                            1,
                            iRenderable->second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            0,
                            1,
                            Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                        );

                        if (iRenderable->second->HasNormalMap())
                        {
                            textureSamplerType = iRenderable->second->GetMaterial(materialIndex)->pNormal->GetSamplerType();
                            m_immediateContext->PSSetShaderResources(
                                1,
                                1,
                                iRenderable->second->GetMaterial(materialIndex)->pNormal->GetTextureResourceView().GetAddressOf()
                            );
                            m_immediateContext->PSSetSamplers(
                                1,
                                1,
                                Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                            );
                        }

                        //Draw with texture
                        m_immediateContext->DrawIndexed(
                            iRenderable->second->GetMesh(i).uNumIndices,
                            iRenderable->second->GetMesh(i).uBaseIndex,
                            iRenderable->second->GetMesh(i).uBaseVertex
                        );
                    }
                }
                else
                {
                    //Draw without texture
                    m_immediateContext->DrawIndexed(iRenderable->second->GetNumIndices(), 0, 0);
                }
            }

            //Render voxels of the main scene
            std::vector<std::shared_ptr<Voxel>> voxel = iScene->second->GetVoxels();
            for (UINT i = 0; i < voxel.size(); i++)
            {
                UINT uStrides[3] = {
                    static_cast<UINT>(sizeof(SimpleVertex)),
                    static_cast<UINT>(sizeof(NormalData)),
                    static_cast<UINT>(sizeof(InstanceData)) };
                UINT uOffsets[3] = { 0u, 0u, 0u };
                ComPtr<ID3D11Buffer> VoxelBuffers[3] = {
                    voxel[i]->GetVertexBuffer(),
                    voxel[i]->GetNormalBuffer(),
                    voxel[i]->GetInstanceBuffer()
                };

                //Set the buffers, and input layout
                m_immediateContext->IASetVertexBuffers(
                    0u,
                    3u,
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
                CBChangesEveryFrame cbChangesEveryFrame = {
                .World = XMMatrixTranspose(voxel[i]->GetWorldMatrix()),
                .OutputColor = voxel[i]->GetOutputColor(),
                .HasNormalMap = voxel[i]->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(
                    voxel[i]->GetConstantBuffer().Get(),
                    0u,
                    nullptr,
                    &cbChangesEveryFrame,
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
                m_immediateContext->VSSetConstantBuffers(
                    3,
                    1,
                    m_cbLights.GetAddressOf()
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

                if (voxel[i]->HasTexture())
                {
                    for (UINT j = 0; j < voxel[i]->GetNumMeshes(); ++j)
                    {
                        UINT materialIndex = voxel[i]->GetMesh(j).uMaterialIndex;
                        assert(materialIndex < voxel[i]->GetNumMaterials());

                        ComPtr<ID3D11ShaderResourceView> aShaderResources[2] =
                        {
                            voxel[i]->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView(),
                            voxel[i]->GetMaterial(materialIndex)->pNormal->GetTextureResourceView()
                        };
                        ComPtr<ID3D11SamplerState> aSamplerStates[2] =
                        { 
                            Texture::s_samplers[static_cast<size_t>(voxel[i]->GetMaterial(materialIndex)->pDiffuse->GetSamplerType())],
                            Texture::s_samplers[static_cast<size_t>(voxel[i]->GetMaterial(materialIndex)->pNormal->GetSamplerType())]                        
                        };
                        m_immediateContext->PSSetShaderResources(
                            0,
                            2,
                            aShaderResources->GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            0,
                            2,
                            aSamplerStates->GetAddressOf()
                        );
                    }
                }

                m_immediateContext->DrawIndexedInstanced(voxel[i]->GetNumIndices(), voxel[i]->GetNumInstances(), 0, 0, 0);
            }

            //Render the models
            for (auto iModel = iScene->second->GetModels().begin(); iModel != iScene->second->GetModels().end(); iModel++)
            {
                UINT aStrides[2] = {
                    static_cast<UINT>(sizeof(SimpleVertex)),
                    static_cast<UINT>(sizeof(NormalData))
                };
                UINT aOffsets[2] = { 0u, 0u };
                ComPtr<ID3D11Buffer> aBuffers[2] =
                {
                    iModel->second->GetVertexBuffer().Get(),
                    iModel->second->GetNormalBuffer().Get()
                };

                //Set the buffers, and input layout
                m_immediateContext->IASetVertexBuffers(
                    0u,
                    2u,
                    aBuffers->GetAddressOf(),
                    aStrides,
                    aOffsets
                );
                m_immediateContext->IASetIndexBuffer(
                    iModel->second->GetIndexBuffer().Get(),
                    DXGI_FORMAT_R16_UINT,
                    0u
                );
                m_immediateContext->IASetInputLayout(iModel->second->GetVertexLayout().Get());

                //Update the constant buffers
                CBChangesEveryFrame cbChangesEveryFrame = {
                    .World = XMMatrixTranspose(iModel->second->GetWorldMatrix()),
                    .OutputColor = iModel->second->GetOutputColor(),
                    .HasNormalMap = iModel->second->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(
                    iModel->second->GetConstantBuffer().Get(),
                    0u,
                    nullptr,
                    &cbChangesEveryFrame,
                    0u,
                    0u
                );

                //Update and bind the constant buffer
                CBSkinning cbSkinning = {
                    .BoneTransforms = {}
                };
                for (UINT i = 0; i < iModel->second->GetBoneTransforms().size(); ++i)
                {
                    cbSkinning.BoneTransforms[i] = XMMatrixTranspose(iModel->second->GetBoneTransforms()[i]);
                }
                m_immediateContext->UpdateSubresource(
                    iModel->second->GetSkinningConstantBuffer().Get(),
                    0u,
                    nullptr,
                    &cbSkinning,
                    0u,
                    0u
                );

                //Set the shaders and their input
                m_immediateContext->VSSetShader(
                    iModel->second->GetVertexShader().Get(),
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
                    iModel->second->GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->VSSetConstantBuffers(
                    3,
                    1,
                    m_cbLights.GetAddressOf()
                );
                m_immediateContext->VSSetConstantBuffers(
                    4,
                    1,
                    iModel->second->GetSkinningConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    0,
                    1,
                    m_camera.GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    2,
                    1,
                    iModel->second->GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    3,
                    1,
                    m_cbLights.GetAddressOf()
                );
                m_immediateContext->PSSetShader(
                    iModel->second->GetPixelShader().Get(),
                    nullptr,
                    0
                );

                if (iModel->second->HasTexture())
                {
                    for (UINT i = 0; i < iModel->second->GetNumMeshes(); ++i)
                    {
                        UINT materialIndex = iModel->second->GetMesh(i).uMaterialIndex;
                        assert(materialIndex < iModel->second->GetNumMaterials());

                        ComPtr<ID3D11ShaderResourceView> aShaderResources[2] =
                        {
                            iModel->second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView(),
                            iModel->second->GetMaterial(materialIndex)->pNormal->GetTextureResourceView()
                        };
                        ComPtr<ID3D11SamplerState> aSamplerStates[2] =
                        {
                            Texture::s_samplers[static_cast<size_t>(iModel->second->GetMaterial(materialIndex)->pDiffuse->GetSamplerType())],
                            Texture::s_samplers[static_cast<size_t>(iModel->second->GetMaterial(materialIndex)->pNormal->GetSamplerType())]
                        };
                        m_immediateContext->PSSetShaderResources(
                            0,
                            2,
                            aShaderResources->GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            0,
                            2,
                            aSamplerStates->GetAddressOf()
                        );

                        //Draw with texture
                        m_immediateContext->DrawIndexed(
                            iModel->second->GetMesh(i).uNumIndices,
                            iModel->second->GetMesh(i).uBaseIndex,
                            iModel->second->GetMesh(i).uBaseVertex
                        );
                    }
                }
                else
                {
                    //Draw without texture
                    m_immediateContext->DrawIndexed(iModel->second->GetNumIndices(), 0, 0);
                }
            }
                
            //Render the skybox
            std::shared_ptr<Skybox> skybox = iScene->second->GetSkyBox();
            if (skybox)
            {
                UINT uStride = static_cast<UINT>(sizeof(SimpleVertex));
                UINT uOffset = 0u;

                m_immediateContext->IASetVertexBuffers(
                    0,
                    1,
                    skybox->GetVertexBuffer().GetAddressOf(),
                    &uStride,
                    &uOffset
                );
                m_immediateContext->IASetIndexBuffer(
                    skybox->GetIndexBuffer().Get(),
                    DXGI_FORMAT_R16_UINT,
                    0
                );
                m_immediateContext->IASetInputLayout(skybox->GetVertexLayout().Get());

                CBChangesEveryFrame cbChangesEveryFrame = {
                    .World = XMMatrixTranspose(skybox->GetWorldMatrix()),
                    .OutputColor = skybox->GetOutputColor(),
                    .HasNormalMap = skybox->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(
                    skybox->GetConstantBuffer().Get(),
                    0,
                    nullptr,
                    &cbChangesEveryFrame,
                    0,
                    0
                );

                m_immediateContext->VSSetShader(
                    skybox->GetVertexShader().Get(),
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
                    skybox->GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    0,
                    1,
                    m_camera.GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    2,
                    1,
                    skybox->GetConstantBuffer().GetAddressOf()
                );
                m_immediateContext->PSSetConstantBuffers(
                    3,
                    1,
                    m_cbLights.GetAddressOf()
                );
                m_immediateContext->PSSetShader(
                    skybox->GetPixelShader().Get(),
                    nullptr,
                    0
                );

                if (skybox->HasTexture())
                {
                    for (UINT i = 0; i < skybox->GetNumMeshes(); i++)
                    {
                        UINT materialIndex = skybox->GetMesh(i).uMaterialIndex;
                        assert(materialIndex < skybox->GetNumMaterials());

                        ComPtr<ID3D11ShaderResourceView> shaderResources = skybox->GetSkyboxTexture()->GetTextureResourceView();
                        eTextureSamplerType textureSamplerType = skybox->GetMaterial(materialIndex)->pDiffuse->GetSamplerType();
                        ComPtr<ID3D11SamplerState> samplerStates = Texture::s_samplers[static_cast<size_t>(textureSamplerType)];

                        m_immediateContext->PSSetShaderResources(
                            0,
                            1,
                            shaderResources.GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            0,
                            1,
                            samplerStates.GetAddressOf()
                        );
                        m_immediateContext->DrawIndexed(
                            skybox->GetMesh(i).uNumIndices,
                            skybox->GetMesh(i).uBaseIndex,
                            skybox->GetMesh(i).uBaseVertex
                        );
                    }
                }
                else
                {
                    m_immediateContext->DrawIndexed(skybox->GetNumIndices(), 0, 0);
                }
            }
        }

        //Present
        m_swapChain->Present(0, 0);
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