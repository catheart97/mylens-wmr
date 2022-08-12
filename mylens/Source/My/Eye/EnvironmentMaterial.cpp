#include "pch.h"

#include "Eye/EnvironmentMaterial.h"

const std::vector<D3D11_INPUT_ELEMENT_DESC> My::Eye::EnvironmentMaterial::ENVIRONMENT_SHADER_LAYOUT_DESC{
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
};