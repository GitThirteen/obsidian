module;
#include <native/macros.h>

// ============================ //
//  MAGMA - DEFINITIONS MODULE  //
// ============================ //

export module Obsidian.Magma.Definitions;

import std;

EXPORT(obsidian)

inline constexpr float PI       = std::numbers::pi_v<float>;
inline constexpr float TWOPI    = PI * 2.0f;
inline constexpr float HALFPI   = PI * 0.5f;
inline constexpr float FOURPI   = PI * 4.0f;
inline constexpr float INVPI    = std::numbers::inv_pi_v<float>;
inline constexpr float E        = std::numbers::e_v<float>;
inline constexpr float SQRT2    = std::numbers::sqrt2_v<float>;
inline constexpr float INVSQRT2 = 0.70710678118654752440f;

inline constexpr float DEG_TO_RAD = PI / 180.0f;
inline constexpr float RAD_TO_DEG = 180.0f / PI;

inline constexpr float EPSILON          = std::numeric_limits<float>::epsilon();
inline constexpr float EPSILON_RELAXED  = 1e-4f;
inline constexpr float INFINITY         = std::numeric_limits<float>::infinity();

inline constexpr double PI_D         = std::numbers::pi_v<double>;
inline constexpr double TWOPI_D      = PI_D * 2.0;
inline constexpr double DEG_TO_RAD_D = PI_D / 180.0;
inline constexpr double RAD_TO_DEG_D = 180.0 / PI_D;

EXPORT_END