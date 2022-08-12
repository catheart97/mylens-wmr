#pragma once

#include "pch.h"

#include "Material.h"

#include "Object.h"
#include "ShaderStructures.h"

namespace My::Eye
{

namespace _implementation
{

using namespace DirectX;

/**
 * @brief   Class representing a 3D mesh.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Mesh : public Object
{
    // Data //
private:
    std::vector<XMFLOAT3> _vertices;
    std::vector<XMFLOAT3> _normals;
    std::vector<XMFLOAT2> _uv;

    std::vector<UINT> _indices;

    std::shared_ptr<Material> _material;

    winrt::com_ptr<ID3D11Buffer> _vertex_buffer{nullptr};
    winrt::com_ptr<ID3D11Buffer> _index_buffer{nullptr};

    winrt::com_ptr<ID3D11Device1> _device{nullptr};

    // Constructors //
public:
    Mesh(const std::vector<XMFLOAT3> & vertices, //
         const std::vector<XMFLOAT3> & normals,  //
         const std::vector<XMFLOAT2> & uv,       //
         const std::vector<UINT> & indices,      //
         std::shared_ptr<Material> material,     //
         winrt::com_ptr<ID3D11Device1> device,   //
         Object * parent = nullptr               //
         )
        : Object(parent), _vertices(vertices), _normals(normals), _uv{uv},
          _indices(indices), _material{material}, _device{device}
    {
        assert(indices.size() == vertices.size());

        // vertex data buffer
        D3D11_BUFFER_DESC v_buffer_desc{0};
        v_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(VERTEX_DATA) * _vertices.size());
        v_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        auto data{make_data()};
        D3D11_SUBRESOURCE_DATA v_sub_data{data.data(), 0, 0};

        winrt::check_hresult(
            device->CreateBuffer(&v_buffer_desc, &v_sub_data, _vertex_buffer.put()));

        // index data buffer
        D3D11_BUFFER_DESC i_buffer_desc{0};
        i_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE; // TODO: Maybe flag?
        i_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * _indices.size());
        i_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA i_sub_data{_indices.data(), 0, 0};
        winrt::check_hresult(
            device->CreateBuffer(&i_buffer_desc, &i_sub_data, _index_buffer.put()));
    }

    // Methods
protected:
    std::vector<VERTEX_DATA> make_data();

public:
    void render(RenderData & data) override
    {
        if (visible())
        {
            data.device_context->RSSetState(wireframe() ? data._wireframe_state
                                                        : data._solid_state);

            _material->bind(data);

            UINT stride{sizeof(VERTEX_DATA)}, offset{0};
            auto * v_buffer = _vertex_buffer.get(); // simulate array
            data.device_context->IASetVertexBuffers(0, 1, &v_buffer, &stride, &offset);
            data.device_context->IASetIndexBuffer(_index_buffer.get(), DXGI_FORMAT_R32_UINT, 0);

            data.device_context->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // TODO: Flag
            // data.device_context->Draw(static_cast<UINT>(_vertices.size()), 0);
            data.device_context->DrawIndexedInstanced(
                static_cast<UINT>(_vertices.size()), // Index count per instance.
                2,                                   // Instance count.
                0,                                   // Start index location.
                0,                                   // Base vertex location.
                0                                    // Start instance location.
            );
        }
    }
};

} // namespace _implementation

using _implementation::Mesh;

} // namespace My::Eye