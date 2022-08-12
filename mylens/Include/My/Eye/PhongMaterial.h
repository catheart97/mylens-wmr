#pragma once

#include "pch.h"

#include "Eye/Material.h"
#include "Eye/ShaderProgram.h"

namespace My::Eye
{

namespace _implementation
{

struct CONSTANT_PS_PHONG
{
    Color ambient;
    Color diffuse;
    Color specular;
    float alpha;
    float shininess;
    XMFLOAT3 camera;
    XMVECTOR lights[MAX_NUMBER_LIGHTS];
};

static_assert((sizeof(CONSTANT_PS_PHONG) % (sizeof(float) * 4)) == 0,
              "Constant buffer size must be 16-byte aligned.");

class PhongMaterial : public Material
{
    // Data
private:
    Color _ambient, _diffuse, _specular;
    float _alpha, _shininess;

    winrt::com_ptr<ID3D11Device1> _device;
    winrt::com_ptr<ID3D11DeviceContext1> _device_context;

    static std::unique_ptr<ShaderProgram> _shader;
    static winrt::com_ptr<ID3D11InputLayout> _input_layout;
    static ID3D11Buffer * _phong_constant_buffer;

    // Constructors / Destructors //
public:
    PhongMaterial(Color ambient, Color diffuse, Color specular, float alpha, float shininess,
                  winrt::com_ptr<ID3D11Device1> device,
                  winrt::com_ptr<ID3D11DeviceContext1> device_context);

    // Methods //
public:
    void bind(RenderData & data) override;
};

} // namespace _implementation

using _implementation::PhongMaterial;

} // namespace My::Eye