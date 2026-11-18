// Suite Rack — factory presets
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>

#include "module_ids.hpp"
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
#define PS(slot, offset) slotParam((slot), (offset))
#define PC(slot, control) slotParam((slot), kSlotOffsetC0 + (control))

inline constexpr PresetValue kInit[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Comp76Blue)),
    PV(PS(2, kSlotOffsetType), 0),
    PV(PS(3, kSlotOffsetType), 0),
    PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0),
    PV(PC(0, kCtlLow), 0.60), PV(PC(0, kCtlLowMid), 0.45),
    PV(PC(0, kCtlHighMid), 0.55), PV(PC(0, kCtlHigh), 0.57),
    PV(PC(0, kCtlHpf), 0.10), PV(PC(0, kCtlLpf), 1.00),
    PV(PC(1, kCtlThreshold), 0.50), PV(PC(1, kCtlRatio), 0.50),
    PV(PC(1, kCtlAttack), 0.25), PV(PC(1, kCtlRelease), 0.35),
    PV(PC(1, kCtlMakeup), 0.80), PV(PC(1, kCtlMix), 1.00),
    PV(PC(1, kCtlScHpf), 0.05),
};

inline constexpr PresetValue kVocal[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(0, kCtlLow), 0.50), PV(PC(0, kCtlLowMid), 0.44),
    PV(PC(0, kCtlHighMid), 0.60), PV(PC(0, kCtlHigh), 0.56),
    PV(PC(0, kCtlHpf), 0.42), PV(PC(0, kCtlLpf), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Comp76Blue)),
    PV(PC(1, kCtlThreshold), 0.40), PV(PC(1, kCtlRatio), 0.50),
    PV(PC(1, kCtlAttack), 0.45), PV(PC(1, kCtlRelease), 0.55),
    PV(PC(1, kCtlMakeup), 0.62), PV(PC(1, kCtlMix), 0.75),
    PV(PC(1, kCtlScHpf), 0.20),
};

inline constexpr PresetValue kDrum[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(0, kCtlLow), 0.60), PV(PC(0, kCtlLowMid), 0.45),
    PV(PC(0, kCtlHighMid), 0.55), PV(PC(0, kCtlHigh), 0.55),
    PV(PC(0, kCtlHpf), 0.02), PV(PC(0, kCtlLpf), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Comp76Black)),
    PV(PC(1, kCtlThreshold), 0.65), PV(PC(1, kCtlRatio), 0.375),
    PV(PC(1, kCtlAttack), 0.25), PV(PC(1, kCtlRelease), 0.35),
    PV(PC(1, kCtlMakeup), 0.80), PV(PC(1, kCtlMix), 1.00),
    PV(PC(1, kCtlScHpf), 0.05),
};

#undef PC
#undef PS
#undef PV

inline constexpr Preset kPresets[] = {
    {"Init", "Init", kInit, static_cast<int>(sizeof(kInit) / sizeof(kInit[0]))},
    {"Vocal", "Vocal Chain", kVocal, static_cast<int>(sizeof(kVocal) / sizeof(kVocal[0]))},
    {"Drum", "Drum Chain", kDrum, static_cast<int>(sizeof(kDrum) / sizeof(kDrum[0]))},
};

inline constexpr int kPresetCount = static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

} // namespace rack
