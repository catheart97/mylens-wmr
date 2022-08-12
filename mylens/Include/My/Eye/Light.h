#pragma once

#include "pch.h"

#include "Object.h"

namespace My::Eye
{

namespace _implementation
{

/**
 * @brief   Class representing a simple point light.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Light : public Object
{
    // Data //
private:
    float _intensity{1};

    // Properties //
public:
    void intensity(float intensity) { _intensity = intensity; }

    float intensity() { return _intensity; }

    // Constructor //
public:
    Light(Object * parent = nullptr) : Object(parent) {}

    // Methods //
public:
    virtual void render(RenderData & data) override {}
};

} // namespace _implementation

using _implementation::Light;

} // namespace My::Eye