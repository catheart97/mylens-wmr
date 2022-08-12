#pragma once
#pragma once

#include "pch.h"

#include "Eye/Mesh.h"
#include "Eye/PBRMaterial.h"
#include "Eye/PhongMaterial.h"

namespace My::Utility
{

namespace _implementation
{

/**
 * @brief   Class containing various utilities and enumerations for winrt/C++ (DirectX)
 * environments.
 *
 * @ingroup Utility
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class winrtUtility
{
public:
    static inline void LogMessage(winrt::hstring str)
    {
        winrt::hstring output(str);
        output = L"LOG: " + output + L"\n";

#ifdef _DEBUG
        winrt::Windows::Foundation::Diagnostics::LoggingChannel log_channel(
            L"MyLens Log"
#ifdef NATIVE_LOGGING_CHANNEL
            ,
            0,
            {0x4bd2826e,
             0x54a1,
             0x4ba9,
             {0xbf, 0x63, 0x92, 0xb7, 0x3e, 0xa1, 0xac, 0x4a}}); // GUID optional
#else
        );
#endif
        log_channel.LogMessage(output);
        OutputDebugString(output.c_str());
#endif
    }

    static std::wstring V2S(const DirectX::XMVECTOR & v)
    {
        using namespace DirectX;

        float x = XMVectorGetByIndex(v, 0);
        float y = XMVectorGetByIndex(v, 1);
        float z = XMVectorGetByIndex(v, 2);
        float w = XMVectorGetByIndex(v, 3);

        std::wstringstream ss;
        ss << L"vector( x: " << x << L" y: " << y << L" z: " << z << L" w: " << w << L" )";
        return ss.str();
    }

    static concurrency::task<std::unordered_map<std::wstring, std::shared_ptr<My::Eye::Material>>>
    load_from_mtl(winrt::Windows::Storage::StorageFile mtl_file,
                  winrt::com_ptr<ID3D11Device1> device,
                  winrt::com_ptr<ID3D11DeviceContext1> device_context)
    {
        using namespace std;
        using namespace winrt;
        using namespace My::Eye;

        unordered_map<wstring, shared_ptr<Material>> result;

        auto content = co_await winrt::Windows::Storage::FileIO::ReadTextAsync(mtl_file);

        wstring s_str(content);
        wstringstream file_ss(s_str);
        wstring line{L""};

        wstring mat_name = L"";
        float Ns, Ka, Ks, Ni, d, illum; // Ke for emmission
        Color Kd;

        auto push_material = [&]() {
            Color albedo = Kd;
            float roughness = 1 - sqrt(Ns) / 30;
            float metalness = illum == 3 || illum == 6 ? Ka : 0.f;
            float ambient = 1.f;
            float ior = Ni;
            float alpha = d;

            auto rs =
                make_pair(mat_name, make_shared<PBRMaterial>(albedo, roughness, metalness, ambient,
                                                             ior, alpha, device, device_context));
            result.insert(rs);
        };

        while (getline(file_ss, line))
        {
            if (line[0] == '#') continue;

            wstringstream ss(line);
            wstring command;
            ss >> command;

            if (command == L"Ka")
                ss >> Ka;
            else if (command == L"Kd")
                ss >> Kd.r >> Kd.g >> Kd.b;
            else if (command == L"Ks")
                ss >> Ks;
            else if (command == L"Ns")
                ss >> Ns;
            else if (command == L"Ni")
                ss >> Ni;
            else if (command == L"d")
                ss >> d;
            else if (command == L"illum")
                ss >> illum;
            else if (command == L"newmtl")
            {
                if (mat_name != L"") push_material();
                ss >> mat_name;
            }
        }
        if (mat_name != L"") push_material();

        co_return result;
    }

    static concurrency::task<std::vector<std::shared_ptr<My::Eye::Mesh>>>
    load_from_obj(winrt::Windows::Storage::StorageFile obj_file,
                  std::unordered_map<std::wstring, std::shared_ptr<My::Eye::Material>> materials,
                  winrt::com_ptr<ID3D11Device1> device,
                  winrt::com_ptr<ID3D11DeviceContext1> device_context)
    {
        using namespace std;
        using namespace winrt;
        using namespace My::Eye;
        using namespace DirectX;

        vector<shared_ptr<Mesh>> results;
        wstring line{L""};

        hstring content = co_await winrt::Windows::Storage::FileIO::ReadTextAsync(obj_file);

        wstring s_str(content);
        wstringstream file_ss(s_str);

        vector<XMFLOAT3> read_vertices;
        vector<XMFLOAT3> read_normals;
        vector<XMFLOAT2> read_uv;

        vector<XMFLOAT3> vertices;
        vector<XMFLOAT3> normals;
        vector<XMFLOAT2> uv;

        shared_ptr<Material> use_material{nullptr};

        auto push_object = [&]() {
            vector<UINT> indices(vertices.size());
            iota(indices.begin(), indices.end(), 0);

            results.push_back(std::make_shared<Mesh>(vertices, normals, uv, indices, use_material,
                                                     device, nullptr));
            vertices.clear();
            normals.clear();
            uv.clear();
        };

        bool reading = false;

        while (getline(file_ss, line))
        {
            if (line[0] == '#') continue; // dump comment line

            wstringstream ss(line);
            wstring command;
            ss >> command;

            if (command == L"mtllib")
            {
            }
            else if (command == L"o")
            {
                // read_uv.clear(); // blender obj exporter continues counting
                // read_vertices.clear();
                // read_normals.clear();
                if (reading) push_object();
                reading = true;
            }
            else if (command == L"v")
            {
                float x, y, z;
                ss >> x >> y >> z;
                read_vertices.push_back(XMFLOAT3{x, y, z});
            }
            else if (command == L"vt")
            {
                float u, v;
                ss >> u >> v;
                read_uv.push_back(XMFLOAT2{u, v});
            }
            else if (command == L"vn")
            {
                float x, y, z;
                ss >> x >> y >> z;
                read_normals.push_back(XMFLOAT3{x, y, z});
            }
            else if (command == L"usemtl")
            {
                wstring materialid;
                ss >> materialid;
                use_material = materials[materialid];
            }
            else if (command == L"s")
            {} // TODO: smooth shading
            else if (command == L"f")
            {
                wstring vtxs;
                struct INDICES
                {
                    size_t v, u, n;
                };
                vector<INDICES> vtx_l;
                getline(ss, vtxs);

                wstring vtx;
                wstringstream vtx_ss(vtxs);
                while (getline(vtx_ss, vtx, L' '))
                {
                    if (vtx == L"") continue;
                    wstringstream vtx_ss(vtx);
                    wstring tmp;
                    INDICES idc;
                    getline(vtx_ss, tmp, L'/');
                    idc.v = stoi(tmp);
                    getline(vtx_ss, tmp, L'/');
                    idc.u = stoi(tmp);
                    getline(vtx_ss, tmp);
                    idc.n = stoi(tmp);
                    vtx_l.push_back(idc);
                }

                if (vtx_l.size() == 3)
                {
                    for (size_t q = 0; q < vtx_l.size(); ++q)
                    {
                        auto idx_s = vtx_l.size() - 1 - q;
                        auto & idx = vtx_l[idx_s];
                        vertices.push_back(read_vertices[idx.v - 1]);
                        uv.push_back(read_uv[idx.u - 1]);
                        normals.push_back(read_normals[idx.n - 1]);
                    }
                }
                else
                    throw std::runtime_error("Utility: Could not parse face.");
            }
        }
        if (vertices.size()) push_object();

        co_return results;
    }
};

} // namespace _implementation

using _implementation::winrtUtility;

} // namespace My::Utility