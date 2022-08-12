#pragma once

#include "pch.h"

#include "Eye/Shader/ShaderConstants.h"

namespace My::Eye
{

namespace _implementation
{

using namespace winrt;
using namespace DirectX;

/**
 * @brief Vertex data layout.
 *
 * Struct describing the default 3 Vertex 3 Normal Layout for the GPU. The Corresponding
 * descriptor is inside of the Material class.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
struct VERTEX_DATA
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 uv;
};

struct VERTEX_DATA_ENVIRONMENT
{
    XMFLOAT3 position;
};

/**
 * @brief   Constant buffer for vertex shaders.
 *
 * Struct describing the layout that all vertex shaders must accept as first (aka 0) constant buffer
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
struct CONSTANT_VS
{
    XMMATRIX model_m;
    XMMATRIX view_m[2];
    XMMATRIX proj_m[2];
    XMMATRIX normal_m;
};

static_assert((sizeof(CONSTANT_VS) % (sizeof(float) * 4)) == 0,
              "Constant buffer size must be 16-byte aligned.");

/**
 * @brief   Struct that is passed from the Renderer Eye to an Object implementation.
 *
 * It contains the CONSTANT_VS instance which describes the constant data each Vertex Shader must
 * supply.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
struct RenderData
{
    CONSTANT_VS constant_all;
    XMVECTOR lights[MAX_NUMBER_LIGHTS];
    XMVECTOR camera{0, 0, 0, 0};
    ID3D11Buffer * constant_buffer{nullptr};
    com_ptr<ID3D11DeviceContext1> device_context{nullptr};
    ID3D11RasterizerState1 * _solid_state{nullptr};
    ID3D11RasterizerState1 * _wireframe_state{nullptr};
    bool _first_call{true};
};

} // namespace _implementation

using _implementation::CONSTANT_VS;
using _implementation::RenderData;
using _implementation::VERTEX_DATA;

} // namespace My::Eye