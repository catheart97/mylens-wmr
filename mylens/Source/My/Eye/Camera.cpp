#include "pch.h"

#include "Eye/Camera.h"

namespace My::Eye
{

Camera::Camera(
    winrt::Windows::Graphics::Holographic::HolographicCamera  holographic_camera)
    : _holographic_camera{holographic_camera}, _is_stereo{holographic_camera.IsStereo()},
      _render_target_size(holographic_camera.RenderTargetSize())
      
{
    _viewport = CD3D11_VIEWPORT(0.f, 0.f, _render_target_size.Width, _render_target_size.Height);
}

void Camera::create_back_buffer(
    ID3D11Device * device,
    winrt::Windows::Graphics::Holographic::HolographicCameraRenderingParameters const &
        camera_parameters)
{
    using namespace DirectX;
    using namespace winrt::Windows::Graphics::Holographic;
    using namespace winrt::Windows::Graphics::DirectX;
    using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

    IDirect3DSurface surface = camera_parameters.Direct3D11BackBuffer();

    winrt::com_ptr<ID3D11Texture2D> camera_back_buffer{nullptr};
    winrt::check_hresult(
        surface.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>()
            ->GetInterface(IID_PPV_ARGS(camera_back_buffer.put()))); // get back buffer from camera

    // Determine if the back buffer has changed.
    if (_back_buffer.get() != camera_back_buffer.get())
    {
        _back_buffer = camera_back_buffer;

        _render_target_view = nullptr;
        winrt::check_hresult(
            device->CreateRenderTargetView(_back_buffer.get(), nullptr, _render_target_view.put()));

        D3D11_TEXTURE2D_DESC back_buffer_desc;
        _back_buffer->GetDesc(&back_buffer_desc);
        _dxgi_format = back_buffer_desc.Format;

        winrt::Windows::Foundation::Size current_size = _holographic_camera.RenderTargetSize();
        if (_render_target_size != current_size)
        {
            _render_target_size = current_size; // if size changed reset
            _depth_stencil_view = nullptr;
        }
    }

    if (_depth_stencil_view == nullptr)
    {
        CD3D11_TEXTURE2D_DESC depth_stencil_desc(
            DXGI_FORMAT_R16_TYPELESS, static_cast<UINT>(_render_target_size.Width),
            static_cast<UINT>(_render_target_size.Height),
            _is_stereo ? 2 : 1, // 2 textures if stereo
            1,                  // single mipmap level
            D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

        _depth_stencil = nullptr;
        winrt::check_hresult(
            device->CreateTexture2D(&depth_stencil_desc, nullptr, _depth_stencil.put()));

        CD3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc(
            _is_stereo ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_D16_UNORM);
        winrt::check_hresult(device->CreateDepthStencilView(
            _depth_stencil.get(), &depth_stencil_view_desc, _depth_stencil_view.put()));
    }
}

std::pair<bool, std::array<std::pair<DirectX::XMMATRIX, DirectX::XMMATRIX>, 2>> Camera::projection_view(
    ID3D11Device * device, ID3D11DeviceContext * device_context,
    winrt::Windows::Graphics::Holographic::HolographicCameraPose const & camera_pose,
    winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const & coordinate_system)
{
    using namespace DirectX;
    using namespace winrt::Windows::Graphics::Holographic;

    std::array<std::pair<XMMATRIX, XMMATRIX>, 2> proj_view;

    auto viewport = camera_pose.Viewport();
    _viewport = CD3D11_VIEWPORT(viewport.X, viewport.Y, viewport.Width, viewport.Height);

    HolographicStereoTransform camera_projection_transform = camera_pose.ProjectionTransform();
    auto view_transform_container = camera_pose.TryGetViewTransform(coordinate_system);

    bool view_transform_aquired = view_transform_container != nullptr;
    if (view_transform_aquired)
    {
        HolographicStereoTransform view_coordinat_system_transform =
            view_transform_container.Value();

        XMMATRIX proj_l, view_l;
        proj_l = XMLoadFloat4x4(&camera_projection_transform.Left);
        view_l = XMLoadFloat4x4(&view_coordinat_system_transform.Left);

        XMMATRIX proj_r, view_r;
        proj_r = XMLoadFloat4x4(&camera_projection_transform.Right);
        view_r = XMLoadFloat4x4(&view_coordinat_system_transform.Right);

        proj_view[0] = std::make_pair(proj_l, view_l);
        proj_view[1] = std::make_pair(proj_r, view_r);

        _frame_pending = true;
        return std::make_pair(true, proj_view);
    }
    _frame_pending = false;
    return std::make_pair(false, proj_view);
}

} // namespace My::Eye