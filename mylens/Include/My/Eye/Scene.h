#pragma once

#include "pch.h"

#include "Eye/Light.h"
#include "Eye/Object.h"

#include "Utility/winrtUtility.h"

#include "Eye/ShaderStructures.h"

namespace My::Eye
{

namespace _implementation
{

/**
 * @brief   Class representing a scene tree.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Scene : public Object
{
    // Data //
protected:
    std::vector<std::shared_ptr<Object>> _content;
    std::vector<std::shared_ptr<Light>> _lights;

    // Properties //
public:
    void light(std::shared_ptr<Light> light) { _lights.push_back(light); }

    std::shared_ptr<Light> light(size_t i) { return _lights[i]; }

    std::shared_ptr<Light> popl(size_t i = -1);

    void push_back(std::shared_ptr<Object> object)
    {
        auto light = std::dynamic_pointer_cast<Light>(object);
        if (light) return;
        _content.push_back(object);
        object->parent(this);
    }

    size_t size(std::shared_ptr<Object> object) const noexcept { return _content.size(); }

    auto begin() { return _content.begin(); }

    auto end() { return _content.end(); }

    std::shared_ptr<Object> & operator[](size_t i) { return _content[i]; }

    std::shared_ptr<Object> pop(size_t i = -1);

    // Constructors //
public:
    Scene(Object * parent = nullptr) : Object(parent) {}

    // Methods //
protected:
    void prepare_lights(RenderData & data)
    {
        size_t i = 0;

        for (; i < min(MAX_NUMBER_LIGHTS, _lights.size()); ++i)
            data.lights[i] = XMVectorSetByIndex(_lights[i]->position(), _lights[i]->intensity(), 3);

        for (; i < MAX_NUMBER_LIGHTS; ++i) data.lights[i] = XMVectorSet(0, 0, 0, 0);
    }

public:
    bool in(std::shared_ptr<Object> object) const;

    void render(RenderData & data) override;
};

} // namespace _implementation

using _implementation::Scene;

} // namespace My::Eye