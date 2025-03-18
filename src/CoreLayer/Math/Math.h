#pragma once

#include "Constant.h"
#include "Distribution.h"
#include "Function.h"
#include "Geometry.h"

inline bool nearZero(float x) { return std::abs(x) < EPSILON; }
