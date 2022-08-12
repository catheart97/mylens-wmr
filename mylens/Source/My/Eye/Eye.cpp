#include "pch.h"

#include "Eye/Eye.h"

void My::Eye::Eye::add_camera(winrt::Windows::Graphics::Holographic::HolographicCamera camera)
{
    std::lock_guard<std::mutex> guard(_camera_lock);
    _cameras[camera.Id()] = std::make_unique<Camera>(camera);
}

void My::Eye::Eye::remove_camera(winrt::Windows::Graphics::Holographic::HolographicCamera camera)
{
    std::lock_guard<std::mutex> guard(_camera_lock);
    _cameras[camera.Id()]->release_back_buffer(_device_context.get());
    _cameras.erase(camera.Id());
}

My::Eye::Eye::Eye()
{
    using namespace winrt::Windows::Foundation::Metadata;

    _can_use_wait_for_next_frame_api = ApiInformation::IsMethodPresent(
        winrt::name_of<HolographicSpace>(), L"WaitForNextFrameReady");
    _can_get_holographic_display_for_camera =
        ApiInformation::IsPropertyPresent(winrt::name_of<HolographicCamera>(), L"Display");
    _can_get_default_holographic_display =
        ApiInformation::IsMethodPresent(winrt::name_of<HolographicDisplay>(), L"GetDefault");

    _can_commit_direct3D11_depth_buffer = ApiInformation::IsMethodPresent(
        winrt::name_of<HolographicCameraRenderingParameters>(), L"CommitDirect3D11DepthBuffer");

    if (_can_get_default_holographic_display)
    {
        HolographicSpace::IsAvailableChanged(std::bind(
            &Eye::OnHolographicDisplayChanged, this, std::placeholders::_1, std::placeholders::_2));
    }

    OnHolographicDisplayChanged(nullptr, nullptr);
}

My::Eye::Eye::~Eye()
{
    for (auto f : _token_release) f();
}

winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceState My::Eye::Eye::popState()
{
    SpatialInteractionSourceState state = _source_state;
    _source_state = nullptr;
    return _source_state;
}

bool My::Eye::Eye::sdk_layers_available()
{
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_NULL, // There is no need to create a real hardware device.
        0,
        Eye::CREATTION_FLAGS, // Check for the SDK layers.
        nullptr,              // Any feature level will do.
        0,
        D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Runtime apps.
        nullptr,           // No need to keep the D3D device reference.
        nullptr,           // No need to know the feature level.
        nullptr            // No need to keep the D3D device context reference.
    );

    return SUCCEEDED(hr);
}

void My::Eye::Eye::initialize()
{
    using namespace winrt::Windows::UI::Core;

    IDXGIAdapter * adapter{nullptr};

    if (_holographic_space) // Get the correct adapter to be able to render inside of the 3D World
    {
        LUID id = {_holographic_space.PrimaryAdapterId().LowPart,
                   _holographic_space.PrimaryAdapterId().HighPart};

        if ((id.HighPart != 0) || (id.LowPart != 0))
        {
            UINT createFlags = 0;

            winrt::com_ptr<IDXGIFactory1> dxgi_factory{nullptr};
            winrt::check_hresult(CreateDXGIFactory2(createFlags, IID_PPV_ARGS(dxgi_factory.put())));

            winrt::com_ptr<IDXGIFactory4> dxgi_factory4{nullptr};
            dxgi_factory4 = dxgi_factory.as<IDXGIFactory4>();

            winrt::check_hresult(dxgi_factory4->EnumAdapterByLuid(id, IID_PPV_ARGS(&adapter)));
        }
    }

    // Device and Context //
    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> device_context;

    const D3D_DRIVER_TYPE driver_type =
        adapter == nullptr ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN;

    winrt::check_hresult(
        D3D11CreateDevice(adapter,     // IDXGIAdapter *
                          driver_type, // Driver Type (Hardware, Software, ...)
                          nullptr,     // Software Module
                          sdk_layers_available() ? Eye::CREATTION_FLAGS : 0, // Flags
                          FEATURE_LEVELS.data(), // Feature Level supported by MyLens
                          static_cast<UINT>(FEATURE_LEVELS.size()), // length of above
                          D3D11_SDK_VERSION,                        // SDK Version (11)
                          device.put(),           // Device pointer (indirect return)
                          &_feature_level,        // Resulting feature level (indirect return)
                          device_context.put())); // Device context pointer (indirect return)

    _device = device.as<ID3D11Device4>();
    _device_context = device_context.as<ID3D11DeviceContext3>();

    My::Utility::winrtUtility::LogMessage(L"Initialized Direct3D Context.");

    winrt::com_ptr<IDXGIDevice1> dxgi_device = _device.as<IDXGIDevice1>();

    winrt::com_ptr<::IInspectable> object;
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(
        dxgi_device.get(), reinterpret_cast<IInspectable **>(winrt::put_abi(object))));
    _native_device = object.as<IDirect3DDevice>();
    _holographic_space.SetDirect3D11Device(_native_device);

    winrt::com_ptr<IDXGIAdapter> dxgi_adapter;
    winrt::check_hresult(dxgi_device->GetAdapter(dxgi_adapter.put()));
    _dxgi_adapter = dxgi_adapter.as<IDXGIAdapter3>();

    // Constant Vertex Buffer //
    CONSTANT_VS constant_all; // Intitial value;
    constant_all.model_m = XMMatrixIdentity();
    constant_all.view_m[0] = XMMatrixIdentity();
    constant_all.view_m[1] = XMMatrixIdentity();
    constant_all.proj_m[0] = XMMatrixIdentity();
    constant_all.proj_m[1] = XMMatrixIdentity();
    constant_all.normal_m = XMMatrixIdentity();

    D3D11_BUFFER_DESC constant_all_desc;
    constant_all_desc.ByteWidth = sizeof(CONSTANT_VS);
    constant_all_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_all_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_all_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_all_desc.MiscFlags = 0;
    constant_all_desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA constant_all_subres_data;
    constant_all_subres_data.pSysMem = &constant_all;
    constant_all_subres_data.SysMemPitch = 0;
    constant_all_subres_data.SysMemSlicePitch = 0;

    winrt::check_hresult(
        _device->CreateBuffer(&constant_all_desc, &constant_all_subres_data, &_constant_buffer));

    _device_context->VSSetConstantBuffers(0, 1, &_constant_buffer);
    Utility::winrtUtility::LogMessage(L"Constant Buffer 0 initialized.");

    D3D11_RASTERIZER_DESC1 rasterizer_state_desc;
    rasterizer_state_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_state_desc.CullMode = D3D11_CULL_BACK;
    rasterizer_state_desc.FrontCounterClockwise = false; // true
    rasterizer_state_desc.DepthBias = 0;
    rasterizer_state_desc.SlopeScaledDepthBias = 0;
    rasterizer_state_desc.DepthBiasClamp = 0;
    rasterizer_state_desc.DepthClipEnable = true;
    rasterizer_state_desc.ScissorEnable = false;
    rasterizer_state_desc.MultisampleEnable = true; // TODO: Multisampling
    rasterizer_state_desc.AntialiasedLineEnable = true;
    rasterizer_state_desc.ForcedSampleCount = 0;

    winrt::check_hresult(
        _device->CreateRasterizerState1(&rasterizer_state_desc, _rasterizer_state_solid.put()));

    rasterizer_state_desc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizer_state_desc.CullMode = D3D11_CULL_NONE;
    winrt::check_hresult(
        _device->CreateRasterizerState1(&rasterizer_state_desc, _rasterizer_state_wf.put()));

    _device_context->RSSetState(_rasterizer_state_solid.get());
    Utility::winrtUtility::LogMessage(L"Rasterizer(s) initialized.");

    Utility::winrtUtility::LogMessage(L"Direct3D 11 initialized.");

    // Initialize gesture detection
    _spatial_interaction_manager = SpatialInteractionManager::GetForCurrentView();
    auto token = _spatial_interaction_manager.SourcePressed(
        bind(&Eye::OnSourcePressed, this, std::placeholders::_1, std::placeholders::_2));
    _token_release.push_back([=]() { _spatial_interaction_manager.SourcePressed(token); });

    // Alpha Blending
    ID3D11BlendState1 * blend_state{nullptr};
    D3D11_BLEND_DESC1 blend_state_desc;
    ZeroMemory(&blend_state_desc, sizeof(D3D11_BLEND_DESC1));
    blend_state_desc.RenderTarget[0].BlendEnable = true;
    blend_state_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_state_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_state_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_state_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_state_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_state_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    winrt::check_hresult(_device->CreateBlendState1(&blend_state_desc, &blend_state));

    _device_context->OMSetBlendState(blend_state, 0, 0xffffffff);

    Utility::winrtUtility::LogMessage(L"Spatial input initialized.");
}

winrt::Windows::Graphics::Holographic::HolographicFrame
My::Eye::Eye::update(winrt::Windows::Graphics::Holographic::HolographicFrame & frame)
{
    using namespace winrt::Windows::UI::Input::Spatial;

    if (_can_use_wait_for_next_frame_api)
    {
        try
        {
            _holographic_space.WaitForNextFrameReady();
        }
        catch (winrt::hresult_not_implemented const &)
        {
            _can_use_wait_for_next_frame_api = false;
        }
    }
    else if (frame)
    {
        frame.WaitForFrameToFinish();
    }

    HolographicFrame next = _holographic_space.CreateNextFrame();
    HolographicFramePrediction prediction = next.CurrentPrediction();

    {
        std::lock_guard<std::mutex> guard(_camera_lock);
        for (HolographicCameraPose const & camera_pose : prediction.CameraPoses())
        {
            HolographicCameraRenderingParameters render_parameters =
                next.GetRenderingParameters(camera_pose);
            Camera * cam = _cameras[camera_pose.HolographicCamera().Id()].get();

            cam->create_back_buffer(_device.get(), render_parameters);
        }
    }

    // TODO: Handle user input (e.g. from gamepad)

    if (_stationary_reference_frame)
    {
        using namespace winrt::Windows::Foundation::Numerics;

        auto pose = SpatialPointerPose::TryGetAtTimestamp(
            _stationary_reference_frame.CoordinateSystem(), prediction.Timestamp());

        if (pose)
        {
            float3 head_pos = pose.Head().Position();
            float3 head_for = pose.Head().ForwardDirection();

            _eye_position = XMVectorSet(head_pos.x, head_pos.y, head_pos.z, 0);
            _eye_forward = XMVectorSet(head_for.x, head_for.y, head_for.z, 0);
        }
    }

    // TODO: Scene updates

    for (HolographicCameraPose const & camera_pose : prediction.CameraPoses())
    {
        HolographicCameraRenderingParameters render_parameters =
            next.GetRenderingParameters(camera_pose);

        if (_stationary_reference_frame != nullptr)
        {

            XMFLOAT3 pos_xm;
            XMStoreFloat3(&pos_xm, XMVECTOR{0, 0, 0, 0});
            winrt::Windows::Foundation::Numerics::float3 pos(pos_xm.x, pos_xm.y, pos_xm.z);
            render_parameters.SetFocusPoint(_stationary_reference_frame.CoordinateSystem(), pos);
        }
    }

    _first_update = true;
    return next;
}

bool My::Eye::Eye::render(winrt::Windows::Graphics::Holographic::HolographicFrame & frame)
{
    if (!_first_update) return false;

    // TODO: Render environment cubemap, lightmaps, etc... (Maybe from Camera?)

    {
        std::lock_guard<std::mutex> guard(_camera_lock);

        frame.UpdateCurrentPrediction();
        HolographicFramePrediction prediction = frame.CurrentPrediction();

        for (HolographicCameraPose const & camera_pose : prediction.CameraPoses())
        {
            // This represents the device-based resources for a HolographicCamera.
            Camera * cam = _cameras[camera_pose.HolographicCamera().Id()].get();

            const auto depthStencilView = cam->stencil_view();

            // Set render targets to the current holographic camera.
            ID3D11RenderTargetView * const targets[1] = {cam->render_target_view()};

            _device_context->OMSetRenderTargets(1, targets, depthStencilView);

            // Clear the back buffer and depth stencil view.
            if (_can_get_holographic_display_for_camera &&
                camera_pose.HolographicCamera().Display().IsOpaque())
            {
                _device_context->ClearRenderTargetView(targets[0], DirectX::Colors::DarkGray);
            }
            else
            {
                _device_context->ClearRenderTargetView(targets[0], DirectX::Colors::Transparent);
            }
            _device_context->ClearDepthStencilView(
                depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            //
            // Notes regarding holographic content:
            //    * For drawing, remember that you have the potential to fill twice as many
            //      pixels in a stereoscopic render target as compared to a non-stereoscopic render
            //      target of the same resolution. Avoid unnecessary or repeated writes to the
            //      same pixel, and only draw holograms that the user can see.
            //    * To help occlude hologram geometry, you can create a depth map using geometry
            //      data obtained via the surface mapping APIs. You can use this depth map to
            //      avoid rendering holograms that are intended to be hidden behind tables,
            //      walls, monitors, and so on.
            //    * On HolographicDisplays that are transparent, black pixels will appear
            //      transparent to the user. On such devices, you should clear the screen to
            //      Transparent as shown above. You should still use alpha blending to draw
            //      semitransparent holograms.
            //

            if (_stationary_reference_frame)
            {
                auto proj_view_res =
                    cam->projection_view(_device.get(), _device_context.get(), camera_pose,
                                         _stationary_reference_frame.CoordinateSystem());
                if (proj_view_res.first) // check if view and proj matrices could be received.
                {
                    if (_scene)
                    {
                        cam->bind(_device_context.get());

                        CONSTANT_VS constant_all;
                        constant_all.model_m = _scene->model_matrix();
                        constant_all.view_m[0] = proj_view_res.second[0].second;
                        constant_all.view_m[1] = proj_view_res.second[1].second;
                        constant_all.proj_m[0] = proj_view_res.second[0].first;
                        constant_all.proj_m[1] = proj_view_res.second[1].first;
                        constant_all.normal_m = XMMatrixIdentity();

                        RenderData data;
                        data.constant_all = constant_all;
                        data.constant_buffer = _constant_buffer;
                        data.device_context = _device_context;
                        data.camera = _eye_position;
                        data._solid_state = _rasterizer_state_solid.get();
                        data._wireframe_state = _rasterizer_state_wf.get();

                        _scene->render(data);

                        if (_can_commit_direct3D11_depth_buffer)
                        {
                            HolographicCameraRenderingParameters render_parameters =
                                frame.GetRenderingParameters(camera_pose);

                            winrt::com_ptr<IDXGIResource1> depth_stencil_resource{
                                cam->stencil_buffer().as<IDXGIResource1>()};

                            winrt::com_ptr<IDXGISurface2> depth_dxgi_surface;
                            winrt::check_hresult(depth_stencil_resource->CreateSubresourceSurface(
                                0, depth_dxgi_surface.put()));
                            winrt::com_ptr<::IInspectable> inspectable_surface;
                            winrt::check_hresult(CreateDirect3D11SurfaceFromDXGISurface(
                                depth_dxgi_surface.get(),
                                reinterpret_cast<IInspectable **>(
                                    winrt::put_abi(inspectable_surface))));

                            IDirect3DSurface interop_surface = inspectable_surface.as<
                                winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface>();

                            render_parameters.CommitDirect3D11DepthBuffer(interop_surface);
                        }

                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void My::Eye::Eye::OnHolographicDisplayChanged(winrt::Windows::Foundation::IInspectable,
                                               winrt::Windows::Foundation::IInspectable)
{
    using namespace winrt::Windows::Perception::Spatial;

    SpatialLocator spatial_locator = nullptr;
    if (_can_get_default_holographic_display)
    {
        HolographicDisplay defaultHolographicDisplay = HolographicDisplay::GetDefault();
        if (defaultHolographicDisplay)
        {
            spatial_locator = defaultHolographicDisplay.SpatialLocator();
        }
    }
    else
    {
        spatial_locator = SpatialLocator::GetDefault();
    }

    if (_spatial_locator != spatial_locator)
    {
        if (_spatial_locator != nullptr)
        {
            _spatial_locator.LocatabilityChanged(_locatability_changed_token);
            _spatial_locator = nullptr;
        }

        _stationary_reference_frame = nullptr;

        if (spatial_locator != nullptr)
        {
            _spatial_locator = spatial_locator;

            _locatability_changed_token = _spatial_locator.LocatabilityChanged(std::bind(
                &Eye::OnLocatabilityChanged, this, std::placeholders::_1, std::placeholders::_2));

            _stationary_reference_frame =
                _spatial_locator.CreateStationaryFrameOfReferenceAtCurrentLocation();
        }
    }
}

void My::Eye::Eye::OnLocatabilityChanged(
    winrt::Windows::Perception::Spatial::SpatialLocator const & sender,
    winrt::Windows::Foundation::IInspectable const & args)
{
    using namespace winrt::Windows::Perception::Spatial;

    switch (sender.Locatability())
    {
        case SpatialLocatability::Unavailable: {
            winrt::hstring message(L"Warning! Positional tracking is " +
                                   std::to_wstring(int(sender.Locatability())) + L".\n");
            OutputDebugStringW(message.data());
        }
        break;

        case SpatialLocatability::PositionalTrackingActivating:
            // The system is preparing to use positional tracking.

        case SpatialLocatability::OrientationOnly:
            // Positional tracking has not been activated.

        case SpatialLocatability::PositionalTrackingInhibited:
            // Positional tracking is temporarily inhibited. User action may be required
            // in order to restore positional tracking.
            break;

        case SpatialLocatability::PositionalTrackingActive:
            // Positional tracking is active. World-locked content can be rendered.
            break;
    }
}

void My::Eye::Eye::OnSourcePressed(
    winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const & sender,
    winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs const & args)
{
    _source_state = args.State();
}

const std::vector<D3D_FEATURE_LEVEL> My::Eye::Eye::FEATURE_LEVELS{
    D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};