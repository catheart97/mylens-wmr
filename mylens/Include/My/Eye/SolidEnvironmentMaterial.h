#pragma once

#include "pch.h"

#include "Eye/EnvironmentMaterial.h"
#include "Eye/ShaderProgram.h"
#include "Eye/SolidMaterial.h"

namespace My::Eye

{
namespace _implementation
{

class SolidEnvironmentMaterial : public EnvironmentMaterial
{
    // Data //
private:
    Color _albedo{.8f, .8f, .8f};
    float _alpha{1.0f};

    winrt::com_ptr<ID3D11Device4> _device;
    winrt::com_ptr<ID3D11DeviceContext3> _device_context;

    static std::unique_ptr<ShaderProgram> _shader;
    static winrt::com_ptr<ID3D11InputLayout> _input_layout;
    static ID3D11Buffer * _solid_constant_buffer;

    // Constructors //
public:
    SolidEnvironmentMaterial(Color albedo, float alpha, winrt::com_ptr<ID3D11Device1> device,
                             winrt::com_ptr<ID3D11DeviceContext1> device_context)
    {
        if (!_shader)
        {
            _shader = std::make_unique<ShaderProgram>(
                "SolidEnvironmentVertexShader.cso", "SolidEnvironmentGeometryShader.cso",
                "SolidEnvironmentPixelShader.cso", device, device_context);
            _shader->bind();
            _input_layout =
                _shader->generate_layout(EnvironmentMaterial::ENVIRONMENT_SHADER_LAYOUT_DESC);

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
    void bind(RenderData & data) override
    {
        _shader->bind();

        CONSTANT_PS_SOLID solid_constant;
        solid_constant.albedo = _albedo;
        solid_constant.alpha = _alpha;

        D3D11_MAPPED_SUBRESOURCE m_subres{0};
        winrt::check_hresult(
            _device_context->Map(_solid_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_subres));
        memcpy(m_subres.pData, &solid_constant, sizeof(CONSTANT_PS_SOLID));
        _device_context->Unmap(_solid_constant_buffer, 0);

        _device_context->IASetInputLayout(_input_layout.get());
    
    }
};

} // namespace _implementation

using _implementation::SolidEnvironmentMaterial;

} // namespace My::Eye