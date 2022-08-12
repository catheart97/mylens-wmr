#include "pch.h"

#include "Eye/SolidMaterial.h"

void My::Eye::SolidMaterial::bind(RenderData & data)
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

ID3D11Buffer * My::Eye::SolidMaterial::_solid_constant_buffer = nullptr;
std::unique_ptr<My::Eye::ShaderProgram> My::Eye::SolidMaterial::_shader = nullptr;
winrt::com_ptr<ID3D11InputLayout> My::Eye::SolidMaterial::_input_layout = nullptr;