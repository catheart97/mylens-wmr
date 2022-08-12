#pragma once

#include "pch.h"

namespace My::Eye
{

namespace _implementation
{

/**
 * @brief   Wrapper for Direct3D (11) Shaders.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class ShaderProgram
{
private:
    winrt::com_ptr<ID3D11Device1> _device;
    winrt::com_ptr<ID3D11DeviceContext1> _device_context;

    winrt::com_ptr<ID3D11VertexShader> _vertex_shader;
    winrt::com_ptr<ID3D11GeometryShader> _geometry_shader;
    winrt::com_ptr<ID3D11PixelShader> _pixel_shader;

    std::vector<byte> _vertex_code;
    std::vector<byte> _geometry_code;
    std::vector<byte> _pixel_code;

    winrt::com_ptr<ID3D11InputLayout> _input_layout;

public:
    ShaderProgram(std::string vertex_shader, std::string geometry_shader,
                  std::string pixel_shader, // TODO: Geometry op
                  winrt::com_ptr<ID3D11Device1> device,
                  winrt::com_ptr<ID3D11DeviceContext1> device_context)
        : _device{device}, _device_context{device_context}
    {
        _vertex_code = ShaderProgram::load_shader_file(vertex_shader);
        _geometry_code = ShaderProgram::load_shader_file(geometry_shader);
        _pixel_code = ShaderProgram::load_shader_file(pixel_shader);

        winrt::check_hresult(
            _device->CreateVertexShader(static_cast<void *>(_vertex_code.data()), //
                                        _vertex_code.size(),                      //
                                        nullptr,                                  //
                                        _vertex_shader.put())                     //
        );

        winrt::check_hresult(
            _device->CreateGeometryShader(static_cast<void *>(_geometry_code.data()), //
                                          _geometry_code.size(),                      //
                                          nullptr,                                    //
                                          _geometry_shader.put())                     //
        );

        winrt::check_hresult(_device->CreatePixelShader(static_cast<void *>(_pixel_code.data()), //
                                                        _pixel_code.size(),                      //
                                                        nullptr,                                 //
                                                        _pixel_shader.put())                     //
        );
    }

    void bind()
    {
        _device_context->VSSetShader(_vertex_shader.get(), nullptr, 0);
        _device_context->GSSetShader(_geometry_shader.get(), nullptr, 0);
        _device_context->PSSetShader(_pixel_shader.get(), nullptr, 0);
    }

    winrt::com_ptr<ID3D11InputLayout> generate_layout(std::vector<D3D11_INPUT_ELEMENT_DESC> desc)
    {
        winrt::check_hresult(_device->CreateInputLayout(desc.data(),                            //
                                                        static_cast<UINT>(desc.size()),         //
                                                        _vertex_code.data(),                    //
                                                        static_cast<UINT>(_vertex_code.size()), //
                                                        _input_layout.put()));
        return _input_layout;
    }

protected:
    static std::vector<byte> load_shader_file(std::string File)
    {
        // open the file
        std::ifstream shader_file(File, std::ios::in | std::ios::binary | std::ios::ate);

        std::vector<byte> data;
        assert(shader_file.is_open());

        int length = static_cast<int>(shader_file.tellg());

        data.resize(length);
        shader_file.seekg(0, std::ios::beg);
        shader_file.read(reinterpret_cast<char *>(data.data()), length);
        shader_file.close();

        return data;
    }
};

} // namespace _implementation

using _implementation::ShaderProgram;

} // namespace My::Eye