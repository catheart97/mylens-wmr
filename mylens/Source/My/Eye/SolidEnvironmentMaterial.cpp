#include "pch.h"

#include "Eye/SolidEnvironmentMaterial.h"

ID3D11Buffer * My::Eye::SolidEnvironmentMaterial::_solid_constant_buffer = nullptr;
std::unique_ptr<My::Eye::ShaderProgram> My::Eye::SolidEnvironmentMaterial::_shader = nullptr;
winrt::com_ptr<ID3D11InputLayout> My::Eye::SolidEnvironmentMaterial::_input_layout = nullptr;