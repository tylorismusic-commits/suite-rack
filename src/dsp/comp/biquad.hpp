// Comp Suite model reused in Drum Suite — biquad.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Compatibility layer over hearth-dsp.
#pragma once

#include <hdsp/biquad.hpp>

namespace comp {
using hdsp::Biquad;
using hdsp::settleK;
} // namespace comp
