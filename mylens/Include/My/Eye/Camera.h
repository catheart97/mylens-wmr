#pragma once

#include "pch.h"

namespace My::Eye
{

namespace _implementation
{

/**
 * @brief   Class wrapping a holographic camera and it's rendering properties/data.
 * 
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Camera
{
    // Data //
private:
    winrt::com_ptr<ID3D11RenderTargetView> _render_target_view{nullptr};
    winrt::com_ptr<ID3D11DepthStencilView> _depth_stencil_view{nullptr};
    winrt::com_ptr<ID3D11Texture2D> _back_buffer{nullptr};
    winrt::com_ptr<ID3D11Texture2D> _depth_stencil{nullptr};

    // Will be loaded from HolographicCamera
    DXGI_FORMAT _dxgi_format{DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT};
    winrt::Windows::Foundation::Size _render_target_size;

    D3D11_VIEWPORT _viewport{0};

    bool _is_stereo = false;
    bool _frame_pending = false;

    winrt::Windows::Graphics::Holographic::HolographicCamera _holographic_camera = nullptr;

    // Properties //
public:
    ID3D11RenderTargetView * render_target_view() const { return _render_target_view.get(); }

    ID3D11DepthStencilView * stencil_view() const { return _depth_stencil_view.get(); }

    ID3D11Texture2D * back_buffer() const { return _back_buffer.get(); }

    winrt::com_ptr<ID3D11Texture2D> stencil_buffer() const { return _depth_stencil; }

    D3D11_VIEWPORT viewport() const { return _viewport; }

    DXGI_FORMAT dxgi_format() const { return _dxgi_format; }

    winrt::Windows::Foundation::Size render_target_size() const & { return _render_target_size; }

    bool stereo() const { return _is_stereo; }

    winrt::Windows::Graphics::Holographic::HolographicCamera const & holographic_camera() const
    {
        return _holographic_camera;
    }

    // Constructors //
public:
    Camera(winrt::Windows::Graphics::Holographic::HolographicCamera holographic_camera);

    // Methods //
public:
    void create_back_buffer(
        ID3D11Device * device,
        winrt::Windows::Graphics::Holographic::HolographicCameraRenderingParameters const &
            camera_parameters);

    void release_back_buffer(ID3D11DeviceContext * device_context)
    {
        // Release camera-specific resources.
        _back_buffer = nullptr;
        _back_buffer = nullptr;
        _depth_stencil = nullptr;
        _render_target_view = nullptr;
        _depth_stencil_view = nullptr;

        ID3D11RenderTargetView * nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {nullptr};
        device_context->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
        device_context->Flush();
    }

    std::pair<bool, std::array<std::pair<DirectX::XMMATRIX, DirectX::XMMATRIX>, 2>> projection_view(
        ID3D11Device * device, ID3D11DeviceContext * device_context,
        winrt::Windows::Graphics::Holographic::HolographicCameraPose const & camera_pose,
        winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const & coordinate_system);

    bool bind(ID3D11DeviceContext * device_context)
    {
        if (!_frame_pending) return false;

        device_context->RSSetViewports(1, &_viewport);

        _frame_pending = false;
        return true;
    }
};

} // namespace _implementation

using _implementation::Camera;

} // namespace My::Eye