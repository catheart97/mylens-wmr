#pragma once

#include "pch.h"

#include "My/Eye/Material.h"

namespace My::Eye

{

namespace _implementation
{

class EnvironmentMaterial : public Material
{
public:
    static const std::vector<D3D11_INPUT_ELEMENT_DESC> ENVIRONMENT_SHADER_LAYOUT_DESC;

    // Constructiors //
public:
    EnvironmentMaterial() : Material() {}

    // Methods // 
public:
    virtual void bind(RenderData & data) = 0;
};

} // namespace _implementation

using _implementation::EnvironmentMaterial;

} // namespace My::Eye