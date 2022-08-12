#pragma once

#include "pch.h"

#include "Eye/Light.h"
#include "Eye/Mesh.h"
#include "Eye/Scene.h"

#include "Eye/Camera.h"

#include "Utility/winrtUtility.h"

#include "ShaderStructures.h"

/**
 * @brief    Eye Renderer Module
 *
 * @defgroup Eye
 */
namespace My::Eye
{

namespace _implementation
{

using namespace winrt;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::UI::Input::Spatial;

/**
 * @brief   Base class of the Eye rendering engine using Direct3D.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Eye
{
    // Constants //
public:
    static const std::vector<D3D_FEATURE_LEVEL> FEATURE_LEVELS;

    static const UINT CREATTION_FLAGS = D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifdef _DEBUG
                                        | D3D11_CREATE_DEVICE_DEBUG;
#else
        ;
#endif

    // Data //
private:
    winrt::com_ptr<ID3D11Device4> _device;
    winrt::com_ptr<ID3D11DeviceContext3> _device_context;

    IDirect3DDevice _native_device;

    winrt::com_ptr<IDXGIAdapter3> _dxgi_adapter;

    winrt::com_ptr<IDXGISwapChain1> _swap_chain;

    winrt::com_ptr<ID3D11RasterizerState1> _rasterizer_state_solid; // culling etc.
    winrt::com_ptr<ID3D11RasterizerState1> _rasterizer_state_wf;    // culling etc.

    ID3D11Buffer * _constant_buffer{nullptr};
    HolographicSpace _holographic_space{nullptr};

    D3D_FEATURE_LEVEL _feature_level{D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1};

    std::mutex _camera_lock;
    std::map<UINT, std::unique_ptr<Camera>> _cameras;

    bool _can_use_wait_for_next_frame_api{false};
    bool _can_get_default_holographic_display{false};
    bool _can_get_holographic_display_for_camera{false};
    bool _can_commit_direct3D11_depth_buffer{false};

    winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference
        _stationary_reference_frame{nullptr};
    winrt::Windows::Perception::Spatial::SpatialLocator _spatial_locator{nullptr};

    std::shared_ptr<Scene> _scene;

    std::vector<std::function<void()>> _token_release;

    XMVECTOR _eye_position{0, 0, 0, 0};
    XMVECTOR _eye_forward{1, 0, 0, 0};
    SpatialInteractionManager _spatial_interaction_manager{nullptr};
    SpatialInteractionSourceState _source_state{nullptr};

    bool _first_update{false};

    winrt::event_token _locatability_changed_token;

    // Properties //
public:
    void scene(std::shared_ptr<Scene> scene) { _scene = scene; }

    std::shared_ptr<Scene> scene() { return _scene; }

    winrt::com_ptr<ID3D11Device1> device() { return _device; }

    winrt::com_ptr<ID3D11DeviceContext1> device_context() { return _device_context; }

    void holographic_space(HolographicSpace space) { _holographic_space = space; }

    void add_camera(HolographicCamera camera);

    void remove_camera(HolographicCamera camera);

    XMVECTOR eye_position() { return _eye_position; }

    // Constructors/Destructors //
public:
    Eye();

    ~Eye();

    // Methods
protected:
    SpatialInteractionSourceState popState();

public:
    bool sdk_layers_available();

    void initialize();

    HolographicFrame update(HolographicFrame & frame);

    bool render(HolographicFrame & frame);

    // Mixed API Callbacks
public:
    void OnHolographicDisplayChanged(winrt::Windows::Foundation::IInspectable,
                                     winrt::Windows::Foundation::IInspectable);

    void OnLocatabilityChanged(winrt::Windows::Perception::Spatial::SpatialLocator const & sender,
                               winrt::Windows::Foundation::IInspectable const & args);

    void OnSourcePressed(SpatialInteractionManager const & sender,
                         SpatialInteractionSourceEventArgs const & args);
};

} // namespace _implementation

using _implementation::Eye;

} // namespace My::Eye