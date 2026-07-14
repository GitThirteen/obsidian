module;
#include <native/macros.h>

// ============================ //
//   MAGMA - TRANSFORM MODULE   //
// ============================ //

export module Obsidian.Magma.Transform;

import std;
import Obsidian.Magma.Vector;
import Obsidian.Magma.Matrix;
import Obsidian.Magma.Rotation;

EXPORT(obsidian, magma)

struct Transform
{
    Vector<float, 3> position = Vector<float, 3>(0.0f, 0.0f, 0.0f);
    Quaternion rotation = Quaternion();
    Vector<float, 3> scale = Vector<float, 3>(1.0f, 1.0f, 1.0f);

    constexpr Transform() = default;

    constexpr Transform(const Vector<float, 3>& position, const Quaternion& rotation, const Vector<float, 3>& scale)
        : position(position), rotation(rotation), scale(scale) {
    }

    constexpr auto to_matrix() const -> Matrix4x4
    {
        Matrix4x4 res = rotation.to_matrix4x4();

        res[0][0] *= scale.x(); res[1][0] *= scale.x(); res[2][0] *= scale.x();
        res[0][1] *= scale.y(); res[1][1] *= scale.y(); res[2][1] *= scale.y();
        res[0][2] *= scale.z(); res[1][2] *= scale.z(); res[2][2] *= scale.z();

        res[0][3] = position.x();
        res[1][3] = position.y();
        res[2][3] = position.z();

        return res;
    }

    constexpr auto to_view_matrix() const -> Matrix4x4
    {
        Matrix4x4 r_inv = rotation.inverse().to_matrix4x4();

        Vector<float, 3> inv_pos(
            -(r_inv[0][0] * position.x() + r_inv[0][1] * position.y() + r_inv[0][2] * position.z()),
            -(r_inv[1][0] * position.x() + r_inv[1][1] * position.y() + r_inv[1][2] * position.z()),
            -(r_inv[2][0] * position.x() + r_inv[2][1] * position.y() + r_inv[2][2] * position.z())
        );

        r_inv[0][3] = inv_pos.x();
        r_inv[1][3] = inv_pos.y();
        r_inv[2][3] = inv_pos.z();

        return r_inv;
    }

    constexpr auto forward() const -> Vector<float, 3>
    {
        return rotation * Vector<float, 3>(0.0f, 0.0f, -1.0f);
    }

    constexpr auto right() const -> Vector<float, 3>
    {
        return rotation * Vector<float, 3>(1.0f, 0.0f, 0.0f);
    }

    constexpr auto up() const -> Vector<float, 3>
    {
        return rotation * Vector<float, 3>(0.0f, 1.0f, 0.0f);
    }

    constexpr auto apply(const Transform& parent) const -> Transform
    {
        return Transform(
            parent.position + (parent.rotation * (position * parent.scale)),
            parent.rotation * rotation,
            parent.scale * scale
        );
    }
};

EXPORT_END