#pragma once

#include "pch.h"

#include "Material.h"

#include "ShaderProgram.h"
#include "ShaderStructures.h"

namespace My::Eye
{

namespace _implementation
{

using namespace DirectX;

struct CONSTANT_PS_PBR
{
    Color albedo;
    float alpha;
    float roughness;
    float ior;
    float ambient_occlusion;
    float metalness;
    XMVECTOR lights[MAX_NUMBER_LIGHTS];
    XMVECTOR camera;
};

static_assert((sizeof(CONSTANT_PS_PBR) % (sizeof(float) * 4)) == 0,
              "Constant buffer size must be 16-byte aligned.");

/**
 * @brief   PBR Shader Material using Cook-Torrance lightning model.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class PBRMaterial : public Material
{
    // Data
private:
    // TODO: Texture bindings
    Color _albedo{0.f, 0.f, .6f};
    float _roughness{.1f};
    float _metalness{.1f};
    float _ambient_occlusion{1.f};
    float _ior{1.5f};
    float _alpha{1.0f};

    winrt::com_ptr<ID3D11Device1> _device;
    winrt::com_ptr<ID3D11DeviceContext1> _device_context;

    static std::unique_ptr<ShaderProgram> _shader;
    static winrt::com_ptr<ID3D11InputLayout> _input_layout;
    static ID3D11Buffer * _pbr_constant_buffer;

    // Constructors / Destructors //
public:
    PBRMaterial(Color albedo, float roughness, float metalness, float ambient_occlusion, float ior,
                float alpha, winrt::com_ptr<ID3D11Device1> device,
                winrt::com_ptr<ID3D11DeviceContext1> device_context);

    // Methods //
public:
    void bind(RenderData & data) override;
};

} // namespace _implementation

using _implementation::PBRMaterial;

} // namespace My::Eye