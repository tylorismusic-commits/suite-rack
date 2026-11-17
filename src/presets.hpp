// Suite Rack — factory presets
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>

#include "params.hpp"

namespace rack {

struct PresetValue { int param; double value; };
struct Preset {
  const char* category;
  const char* name;
  const PresetValue* values;
  int count;
};

#define PV(p, v) {static_cast<int>(p), (v)}

inline constexpr PresetValue kInit[] = {
    PV(P_POWER, 1), PV(P_INPUT, 0), PV(P_OUTPUT, 0),
    PV(P_SLOT0_TYPE, 1), PV(P_SLOT1_TYPE, 2)};

inline constexpr PresetValue kVocal[] = {
    PV(P_POWER, 1), PV(P_SLOT0_TYPE, 1), PV(P_SLOT0_EQ_HPF, 0.42),
    PV(P_SLOT0_EQ_HIGH_MID, 0.60), PV(P_SLOT1_TYPE, 2),
    PV(P_SLOT1_COMP_THRESHOLD, -24), PV(P_SLOT1_COMP_RATIO, 4),
    PV(P_SLOT1_COMP_ATTACK, 0.45), PV(P_SLOT1_COMP_RELEASE, 0.55),
    PV(P_SLOT1_COMP_MAKEUP, 0.62), PV(P_SLOT1_COMP_MIX, 0.75)};

inline constexpr PresetValue kDrum[] = {
    PV(P_POWER, 1), PV(P_SLOT0_TYPE, 1), PV(P_SLOT0_EQ_LOW, 0.60),
    PV(P_SLOT0_EQ_LOW_MID, 0.45), PV(P_SLOT1_TYPE, 2),
    PV(P_SLOT1_COMP_THRESHOLD, -14), PV(P_SLOT1_COMP_RATIO, 3),
    PV(P_SLOT1_COMP_ATTACK, 0.25), PV(P_SLOT1_COMP_RELEASE, 0.35),
    PV(P_SLOT1_COMP_MAKEUP, 0.80), PV(P_SLOT1_COMP_MIX, 1.0)};

#undef PV

inline constexpr Preset kPresets[] = {
    {"Init", "Init", kInit, static_cast<int>(sizeof(kInit) / sizeof(kInit[0]))},
    {"Vocal", "Vocal Chain", kVocal, static_cast<int>(sizeof(kVocal) / sizeof(kVocal[0]))},
    {"Drum", "Drum Chain", kDrum, static_cast<int>(sizeof(kDrum) / sizeof(kDrum[0]))},
};

inline constexpr int kPresetCount = static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

} // namespace rack
