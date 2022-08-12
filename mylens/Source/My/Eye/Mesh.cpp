#include "pch.h"

#include "Eye/Mesh.h"

std::vector<My::Eye::VERTEX_DATA> My::Eye::Mesh::make_data()
{
    std::vector<VERTEX_DATA> data(_vertices.size());
    for (size_t i = 0; i < _vertices.size(); ++i)
    {
        data[i] = {_vertices[i], _normals[i],
                   _uv.size() == _vertices.size() ? _uv[i] : XMFLOAT2{0, 0}};
    }
    return data;
}
