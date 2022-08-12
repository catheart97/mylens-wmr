#pragma once

#include "pch.h"

#include "Eye/Material.h"
#include "Eye/Object.h"

namespace My::Eye
{

namespace _implementation
{

using namespace DirectX;

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::MixedReality;
using namespace winrt::Microsoft::MixedReality::SceneUnderstanding;

class EnvironmentMesh : public Object
{
    // Data //
private:
    SceneMesh _mesh;
    std::shared_ptr<EnvironmentMaterial> _material{nullptr};
    winrt::com_ptr<ID3D11Device1> _device;

    winrt::com_ptr<ID3D11Buffer> _vertex_buffer{nullptr};
    winrt::com_ptr<ID3D11Buffer> _index_buffer{nullptr};

    // Constructors //
public:
    EnvironmentMesh(SceneMesh mesh, std::shared_ptr<EnvironmentMaterial> material,
                    winrt::com_ptr<ID3D11Device1> device)
        : Object(), _mesh{mesh}, _material{material}, _device{device}
    {

        D3D11_BUFFER_DESC v_buffer_desc{0};
        v_buffer_desc.ByteWidth =
            static_cast<UINT>(sizeof(VERTEX_DATA_ENVIRONMENT) * mesh.VertexCount());
        v_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        auto data = make_data();
        D3D11_SUBRESOURCE_DATA v_sub_data{data.data(), 0, 0};

        winrt::check_hresult(
            device->CreateBuffer(&v_buffer_desc, &v_sub_data, _vertex_buffer.put()));

        // index data buffer
        winrt::array_view<uint32_t> idata;
        _mesh.GetTriangleIndices(idata);

        D3D11_BUFFER_DESC i_buffer_desc{0};
        i_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE; // TODO: Maybe flag?
        i_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * idata.size());
        i_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA i_sub_data{idata.data(), 0, 0};
        winrt::check_hresult(
            device->CreateBuffer(&i_buffer_desc, &i_sub_data, _index_buffer.put()));
    }

    // Methods //
protected:
    std::vector<VERTEX_DATA_ENVIRONMENT> make_data()
    {
        size_t n = _mesh.VertexCount();

        std::vector<VERTEX_DATA_ENVIRONMENT> result(n);
        winrt::array_view<winrt::Windows::Foundation::Numerics::float3> data;

        _mesh.GetVertexPositions(data);

        for (size_t i = 0; i < n; ++i)
        {
            // why are there so much float3 types
            XMStoreFloat3(&(result[i].position), XMLoadFloat3(data.cbegin() + i));
        }

        return result;
    }
};

} // namespace _implementation

using _implementation::EnvironmentMesh;

} // namespace My::Eye