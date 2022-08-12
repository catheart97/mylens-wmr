#include "pch.h"

#include "Eye/Material.h"

const std::vector<D3D11_INPUT_ELEMENT_DESC> My::Eye::Material::DEFAULT_SHADER_LAYOUT_DESC{
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA,
     0},
    {"UV", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 5 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA,
     0},
};