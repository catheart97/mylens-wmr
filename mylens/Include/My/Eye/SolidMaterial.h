#pragma once

#include "pch.h"

#include "Eye/Material.h"
#include "Eye/ShaderProgram.h"

namespace My::Eye
{

namespace _implementation
{

struct CONSTANT_PS_SOLID
{
    Color albedo;
    float alpha;
};

static_assert((sizeof(CONSTANT_PS_SOLID) % (sizeof(float) * 4)) == 0,
              "Constant buffer size must be 16-byte aligned.");


/**
 * @brief   Material with a solid color (and alpha). No lights are used.
 * 
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class SolidMaterial : public Material
{
    // Data
private:
    Color _albedo{1., 1., 1.};
    float _alpha{1.0};

    winrt::com_ptr<ID3D11Device4> _device;
    winrt::com_ptr<ID3D11DeviceContext3> _device_context;

    static std::unique_ptr<ShaderProgram> _shader;
    static winrt::com_ptr<ID3D11InputLayout> _input_layout;
    static ID3D11Buffer * _solid_constant_buffer;

    // Constructors / Destructors //
public:
    SolidMaterial(Color albedo, float alpha, winrt::com_ptr<ID3D11Device1> device,
                  winrt::com_ptr<ID3D11DeviceContext1> device_context)
    {
        if (!_shader)
        {
            _shader =
                std::make_unique<ShaderProgram>("SolidVertexShader.cso", "SolidGeometryShader.cso",
                                                "SolidPixelShader.cso", device, device_context);
            _shader->bind();
            _input_layout = _shader->generate_layout(Material::DEFAULT_SHADER_LAYOUT_DESC);

            CONSTANT_PS_SOLID solid_constant{0};

            D3D11_BUFFER_DESC constant_desc;
            constant_desc.ByteWidth = sizeof(CONSTANT_PS_SOLID);
            constant_desc.Usage = D3D11_USAGE_DYNAMIC;
            constant_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            constant_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            constant_desc.MiscFlags = 0;
            constant_desc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA constant_all_subres_data;
            constant_all_subres_data.pSysMem = &solid_constant;
            constant_all_subres_data.SysMemPitch = 0;
            constant_all_subres_data.SysMemSlicePitch = 0;

            winrt::check_hresult(_device->CreateBuffer(&constant_desc, &constant_all_subres_data,
                                                       &_solid_constant_buffer));

            _device_context->PSSetConstantBuffers(0, 1, &_solid_constant_buffer);
        }
    }

    // Methods //
public:
    void bind(RenderData & data) override;
};

} // namespace _implementation

using _implementation::SolidMaterial;

} // namespace My::Eye