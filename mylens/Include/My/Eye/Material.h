#pragma once

#include "pch.h"

#include "ShaderStructures.h"

namespace My::Eye
{

namespace _implementation
{

/**
 * @brief   Color struct representing rgb values.
 * 
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
*/
struct Color
{
    float r{0.f}, g{0.f}, b{0.f};
};

/**
 * @brief   Material interface for Eye renderer. Use ShaderProgramm to create the underlying
 *          shaders.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Material
{
public:
    /** @brief Shader Layout for Vertex-Normal (6 floats) used by most Shaders. */
    static const std::vector<D3D11_INPUT_ELEMENT_DESC> DEFAULT_SHADER_LAYOUT_DESC;

    // Constructors //
public:
    Material() {}

    virtual ~Material() = default;

    // Methods //
public:
    /**
     * @brief Binds the Material (Shader) to be active. Initializes constant buffer.
     */
    virtual void bind(RenderData & data) = 0;
};

} // namespace _implementation

using _implementation::Color;
using _implementation::Material;

} // namespace My::Eye