#include "pch.h"

#include "Eye/Scene.h"

std::shared_ptr<My::Eye::Object> My::Eye::Scene::pop(size_t i)
{
    if (i == -1)
    {
        auto obj = _content[_content.size() - 1];
        _content.erase(_content.begin() + _content.size() - 1);
        obj->parent(nullptr);
        return obj;
    }
    else
    {
        auto obj = _content[i];
        _content.erase(_content.begin() + i, _content.begin() + i + 1);
        obj->parent(nullptr);
        return obj;
    }
}

std::shared_ptr<My::Eye::Light> My::Eye::Scene::popl(size_t i)
{
    if (i == -1)
    {
        auto lgt = _lights[_lights.size() - 1];
        _lights.erase(_lights.begin() + _lights.size() - 1);
        lgt->parent(nullptr);
        return lgt;
    }
    else
    {
        auto lgt = _lights[i];
        _lights.erase(_lights.begin() + i, _lights.begin() + i + 1);
        lgt->parent(nullptr);
        return lgt;
    }
}

bool My::Eye::Scene::in(std::shared_ptr<Object> object) const
{
    for (size_t i = 0; i < _content.size(); ++i)
    {
        std::shared_ptr scene{std::dynamic_pointer_cast<Scene>(_content[i])};
        if ((scene && scene->in(object)) || _content[i] == object) return true;
    }
    return false;
}

void My::Eye::Scene::render(RenderData & data)
{
    if (data._first_call)
    {
        prepare_lights(data);

        data._first_call = false;
    }

    for (std::shared_ptr<Object> & object : *this)
    {
        // TODO: Not calling render on camera and light classes
        XMMATRIX model = data.constant_all.model_m;
        data.constant_all.model_m = XMMatrixMultiply(model, object->model_matrix());
        XMVECTOR det{0};
        data.constant_all.normal_m = XMMATRIX(XMMatrixTranspose(XMMatrixInverse(&det, data.constant_all.model_m)));
        data.constant_all.normal_m.r[3] = XMVECTOR{0, 0, 0, 1};
        data.constant_all.normal_m.r[0].m128_f32[3] = 0;
        data.constant_all.normal_m.r[1].m128_f32[3] = 0;
        data.constant_all.normal_m.r[2].m128_f32[3] = 0;

        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE m_subres{0};
        winrt::check_hresult(data.device_context->Map(data.constant_buffer, 0,
                                                      D3D11_MAP_WRITE_DISCARD, 0, &m_subres));
        memcpy(m_subres.pData, &data, sizeof(CONSTANT_VS));
        data.device_context->Unmap(data.constant_buffer, 0);

        object->render(data);
        data.constant_all.model_m = model;
    }
}
