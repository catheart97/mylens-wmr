#include "pch.h"

#include "Eye/PBRMaterial.h"

My::Eye::PBRMaterial::PBRMaterial(Color albedo, float roughness, float metalness,
                                  float ambient_occlusion, float ior, float alpha,
                                  winrt::com_ptr<ID3D11Device1> device,
                                  winrt::com_ptr<ID3D11DeviceContext1> device_context)
    : Material(), _albedo{albedo}, _roughness{roughness}, _metalness{metalness},
      _ambient_occlusion{ambient_occlusion}, _ior{ior}, _alpha{alpha}, _device{device},
      _device_context{device_context}
{
    if (!_shader)
    {
        _shader = std::make_unique<ShaderProgram>("PBRVertexShader.cso", "PBRGeometryShader.cso",
                                                  "PBRPixelShader.cso", device,
                                                  device_context); // Auto Compile

        _shader->bind();
        _input_layout = _shader->generate_layout(Material::DEFAULT_SHADER_LAYOUT_DESC);

        // Constant buffer //
        CONSTANT_PS_PBR pbr_constant{0};

        D3D11_BUFFER_DESC constant_desc;
        constant_desc.ByteWidth = sizeof(CONSTANT_PS_PBR);
        constant_desc.Usage = D3D11_USAGE_DYNAMIC;
        constant_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constant_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constant_desc.MiscFlags = 0;
        constant_desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA constant_all_subres_data;
        constant_all_subres_data.pSysMem = &pbr_constant;
        constant_all_subres_data.SysMemPitch = 0;
        constant_all_subres_data.SysMemSlicePitch = 0;

        winrt::check_hresult(_device->CreateBuffer(&constant_desc, &constant_all_subres_data,
                                                   &_pbr_constant_buffer));

        _device_context->PSSetConstantBuffers(0, 1, &_pbr_constant_buffer);
    }
}

void My::Eye::PBRMaterial::bind(My::Eye::RenderData & data)
{
    _shader->bind();

    CONSTANT_PS_PBR pbr_constant;
    pbr_constant.albedo = _albedo;
    for (size_t i = 0; i < MAX_NUMBER_LIGHTS; ++i) pbr_constant.lights[i] = data.lights[i];
    pbr_constant.alpha = _alpha;
    pbr_constant.ambient_occlusion = _ambient_occlusion;
    pbr_constant.ior = _ior;
    pbr_constant.metalness = _metalness;
    pbr_constant.roughness = _roughness;
    pbr_constant.camera = data.camera;

    D3D11_MAPPED_SUBRESOURCE m_subres{0};
    winrt::check_hresult(
        _device_context->Map(_pbr_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_subres));
    memcpy(m_subres.pData, &pbr_constant, sizeof(CONSTANT_PS_PBR));
    _device_context->Unmap(_pbr_constant_buffer, 0);

    _device_context->IASetInputLayout(_input_layout.get());
}

ID3D11Buffer * My::Eye::PBRMaterial::_pbr_constant_buffer = nullptr;
std::unique_ptr<My::Eye::ShaderProgram> My::Eye::PBRMaterial::_shader = nullptr;
winrt::com_ptr<ID3D11InputLayout> My::Eye::PBRMaterial::_input_layout = nullptr;