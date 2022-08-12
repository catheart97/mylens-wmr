#include "pch.h"

#include "Eye/PhongMaterial.h"

My::Eye::PhongMaterial::PhongMaterial(Color ambient, Color diffuse, Color specular, float alpha,
                                      float shininess, winrt::com_ptr<ID3D11Device1> device,
                                      winrt::com_ptr<ID3D11DeviceContext1> device_context)
    : _ambient{ambient}, _diffuse{diffuse}, _specular{specular}, _alpha{alpha},
      _shininess{shininess}, _device{device}, _device_context{device_context}
{
    if (!_shader)
    {
        _shader = std::make_unique<ShaderProgram>("PhongVertexShader.cso", "PhongGeometryShader.cso",
                                                  "PhongPixelShader.cso", device,
                                                  device_context); // Auto Compile

        _shader->bind();
        _input_layout = _shader->generate_layout(Material::DEFAULT_SHADER_LAYOUT_DESC);

        // Constant buffer //
        CONSTANT_PS_PHONG phong_constant{0};

        D3D11_BUFFER_DESC constant_desc;
        constant_desc.ByteWidth = sizeof(CONSTANT_PS_PHONG);
        constant_desc.Usage = D3D11_USAGE_DYNAMIC;
        constant_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constant_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constant_desc.MiscFlags = 0;
        constant_desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA constant_all_subres_data;
        constant_all_subres_data.pSysMem = &phong_constant;
        constant_all_subres_data.SysMemPitch = 0;
        constant_all_subres_data.SysMemSlicePitch = 0;

        winrt::check_hresult(_device->CreateBuffer(&constant_desc, &constant_all_subres_data,
                                                   &_phong_constant_buffer));

        _device_context->PSSetConstantBuffers(0, 1, &_phong_constant_buffer);
    }
}

void My::Eye::PhongMaterial::bind(RenderData & data)
{
    _shader->bind();

    CONSTANT_PS_PHONG phong_constant;
    phong_constant.ambient = _ambient;
    phong_constant.alpha = _alpha;
    for (size_t i = 0; i < MAX_NUMBER_LIGHTS; ++i) phong_constant.lights[i] = data.lights[i];
    phong_constant.shininess = _shininess;
    XMStoreFloat3(&phong_constant.camera, data.camera);
    phong_constant.specular = _specular;
    phong_constant.diffuse = _diffuse;

    D3D11_MAPPED_SUBRESOURCE m_subres{0};
    winrt::check_hresult(
        _device_context->Map(_phong_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_subres));
    memcpy(m_subres.pData, &phong_constant, sizeof(CONSTANT_PS_PHONG));
    _device_context->Unmap(_phong_constant_buffer, 0);

    _device_context->IASetInputLayout(_input_layout.get());
}

ID3D11Buffer * My::Eye::PhongMaterial::_phong_constant_buffer = nullptr;
std::unique_ptr<My::Eye::ShaderProgram> My::Eye::PhongMaterial::_shader = nullptr;
winrt::com_ptr<ID3D11InputLayout> My::Eye::PhongMaterial::_input_layout = nullptr;