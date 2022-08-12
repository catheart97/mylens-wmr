#pragma once

#include "pch.h"

#include "ShaderStructures.h"

namespace My::Eye
{

namespace _implementation
{

using namespace DirectX;

/**
 * @brief   Interface for a Scene-Object.
 *
 * @ingroup Eye
 * @author  Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Object
{
    // Data //
protected:
    Object * _parent;

    XMVECTOR _rotation_q{XMQuaternionIdentity()};
    XMVECTOR _position_v{0.f, 0.f, 0.f, 0.f};
    XMVECTOR _scale_v{1.f, 1.f, 1.f, 0.f}; // fourth coordinate is ignored.

    bool _visible{true};
    bool _wireframe{false};

    // Properties //
public:
    void parent(Object * parent) { _parent = parent; }

    Object * parent() { return _parent; }

    XMVECTOR rotation() { return _rotation_q; }

    void rotation(const XMVECTOR & rotation_q) { _rotation_q = XMQuaternionNormalize(rotation_q); }

    XMVECTOR position() { return _position_v; }

    void position(const XMVECTOR & position_v) { _position_v = position_v; }

    XMVECTOR scale() { return _scale_v; }

    void scale(const XMVECTOR & scale_v) { _scale_v = XMVectorSetByIndex(scale_v, 0, 3); }

    void scale_x(float v) { _scale_v = XMVectorSetByIndex(_scale_v, v, 0); }

    void scale_y(float v) { _scale_v = XMVectorSetByIndex(_scale_v, v, 1); }

    void scale_z(float v) { _scale_v = XMVectorSetByIndex(_scale_v, v, 2); }

    void visible(bool v) { _visible = v; }

    void wireframe(bool v) { _wireframe = v; }

    bool wireframe() { return _wireframe; }

    bool visible() { return _visible; }

    virtual XMMATRIX model_matrix()
    {
        return XMMatrixAffineTransformation(_scale_v, XMVECTOR{0.f, 0.f, 0.f, 0.f}, _rotation_q,
                                            _position_v);
    }

    // Constructors //
public:
    Object() : _parent{nullptr} {}
    Object(Object * parent) : _parent{parent} {}

    virtual ~Object() = default;

    // Methods //
public:
    virtual void render(RenderData & data) = 0;
};

} // namespace _implementation

using _implementation::Object;

} // namespace My::Eye