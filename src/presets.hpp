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


inline constexpr PresetValue kMixBusGlue[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(0, kCtlLow), 0.55), PV(PC(0, kCtlLowMid), 0.48),
    PV(PC(0, kCtlHighMid), 0.52), PV(PC(0, kCtlHigh), 0.54),
    PV(PC(0, kCtlHpf), 0.15), PV(PC(0, kCtlLpf), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Glue)),
    PV(PC(1, 0), 0.45),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::JcL2)),
    PV(PC(2, 0), 0.80), PV(PC(2, 1), 0.95), PV(PC(2, 2), 1.00),
    PV(PC(2, 3), 0.50), PV(PC(2, 4), 1.00), PV(PC(2, 5), 0.50),
    PV(PC(2, 6), 0.667),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0),
};

inline constexpr PresetValue kBassDi[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(0, kCtlLow), 0.62), PV(PC(0, kCtlLowMid), 0.55),
    PV(PC(0, kCtlHighMid), 0.45), PV(PC(0, kCtlHigh), 0.48),
    PV(PC(0, kCtlHpf), 0.05), PV(PC(0, kCtlLpf), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Comp76Blue)),
    PV(PC(1, kCtlThreshold), 0.58), PV(PC(1, kCtlRatio), 0.375),
    PV(PC(1, kCtlAttack), 0.45), PV(PC(1, kCtlRelease), 0.50),
    PV(PC(1, kCtlMakeup), 0.55), PV(PC(1, kCtlMix), 1.00),
    PV(PC(1, kCtlScHpf), 0.02),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Tame)),
    PV(PC(2, 0), 0.20), PV(PC(2, 1), 0.50), PV(PC(2, 2), 0.50),
    PV(PC(2, 3), 0.40), PV(PC(2, 4), 0.30),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0),
};

inline constexpr PresetValue kVocalAir[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(0, kCtlLow), 0.48), PV(PC(0, kCtlLowMid), 0.45),
    PV(PC(0, kCtlHighMid), 0.55), PV(PC(0, kCtlHigh), 0.58),
    PV(PC(0, kCtlHpf), 0.45), PV(PC(0, kCtlLpf), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Comp2A)),
    PV(PC(1, 0), 0.45), PV(PC(1, 4), 0.55), PV(PC(1, 5), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::SpacePlate)),
    PV(PC(2, 0), 0.45), PV(PC(2, 1), 0.162), PV(PC(2, 2), 0.10),
    PV(PC(2, 3), 0.55), PV(PC(2, 4), 0.65), PV(PC(2, 5), 0.28),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::Tame)),
    PV(PC(3, 0), 0.35), PV(PC(3, 1), 0.60), PV(PC(3, 2), 0.55),
    PV(PC(3, 3), 0.55), PV(PC(3, 4), 1.00),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
};

inline constexpr PresetValue kDrumRoom[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(0, kCtlLow), 0.60), PV(PC(0, kCtlLowMid), 0.45),
    PV(PC(0, kCtlHighMid), 0.55), PV(PC(0, kCtlHigh), 0.55),
    PV(PC(0, kCtlHpf), 0.02), PV(PC(0, kCtlLpf), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Comp76Black)),
    PV(PC(1, kCtlThreshold), 0.55), PV(PC(1, kCtlRatio), 0.50),
    PV(PC(1, kCtlAttack), 0.55), PV(PC(1, kCtlRelease), 0.30),
    PV(PC(1, kCtlMakeup), 0.40), PV(PC(1, kCtlMix), 0.45),
    PV(PC(1, kCtlScHpf), 0.05),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::BleedOut)),
    PV(PC(2, 0), 0.714), PV(PC(2, 1), 0.366), PV(PC(2, 2), 0.15),
    PV(PC(2, 3), 0.50), PV(PC(2, 4), 0.00),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::SpaceRoom)),
    PV(PC(3, 0), 0.40), PV(PC(3, 1), 0.060), PV(PC(3, 2), 0.04),
    PV(PC(3, 3), 0.55), PV(PC(3, 4), 0.45), PV(PC(3, 5), 0.20),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
};

inline constexpr PresetValue kMasteringChain[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(0, kCtlLow), 0.52), PV(PC(0, kCtlLowMid), 0.49),
    PV(PC(0, kCtlHighMid), 0.51), PV(PC(0, kCtlHigh), 0.54),
    PV(PC(0, kCtlHpf), 0.10), PV(PC(0, kCtlLpf), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::HearthComp)),
    PV(PC(1, 0), 0.50),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::JcL2)),
    PV(PC(2, 0), 0.85), PV(PC(2, 1), 0.97), PV(PC(2, 2), 1.00),
    PV(PC(2, 3), 0.40), PV(PC(2, 4), 1.00), PV(PC(2, 5), 0.50),
    PV(PC(2, 6), 0.667),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::JcO3)),
    PV(PC(3, 0), 0.80), PV(PC(3, 1), 0.90), PV(PC(3, 2), 0.40),
    PV(PC(3, 3), 1.00),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
};

inline constexpr PresetValue kSpaceVocal[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Comp76Blue)),
    PV(PC(0, kCtlThreshold), 0.55), PV(PC(0, kCtlRatio), 0.25),
    PV(PC(0, kCtlAttack), 0.25), PV(PC(0, kCtlRelease), 0.55),
    PV(PC(0, kCtlMakeup), 0.60), PV(PC(0, kCtlMix), 1.00),
    PV(PC(0, kCtlScHpf), 0.20),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(1, kCtlLow), 0.50), PV(PC(1, kCtlLowMid), 0.45),
    PV(PC(1, kCtlHighMid), 0.58), PV(PC(1, kCtlHigh), 0.60),
    PV(PC(1, kCtlHpf), 0.50), PV(PC(1, kCtlLpf), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::SpaceHall)),
    PV(PC(2, 0), 0.70), PV(PC(2, 1), 0.359), PV(PC(2, 2), 0.175),
    PV(PC(2, 3), 0.45), PV(PC(2, 4), 0.80), PV(PC(2, 5), 0.30),
    PV(PS(3, kSlotOffsetType), static_cast<int>(ModuleId::Tame)),
    PV(PC(3, 0), 0.30), PV(PC(3, 1), 0.60), PV(PC(3, 2), 0.55),
    PV(PC(3, 3), 0.50), PV(PC(3, 4), 0.50),
    PV(PS(4, kSlotOffsetType), 0), PV(PS(5, kSlotOffsetType), 0),
};

inline constexpr PresetValue kGuitarSpring[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(0, kCtlLow), 0.52), PV(PC(0, kCtlLowMid), 0.48),
    PV(PC(0, kCtlHighMid), 0.55), PV(PC(0, kCtlHigh), 0.57),
    PV(PC(0, kCtlHpf), 0.15), PV(PC(0, kCtlLpf), 1.00),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Comp2A)),
    PV(PC(1, 0), 0.35), PV(PC(1, 4), 0.55), PV(PC(1, 5), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::SpaceSpring)),
    PV(PC(2, 0), 0.55), PV(PC(2, 1), 0.145), PV(PC(2, 2), 0.00),
    PV(PC(2, 3), 0.50), PV(PC(2, 4), 0.60), PV(PC(2, 5), 0.30),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0),
};

inline constexpr PresetValue kParallelSmash[] = {
    PV(P_POWER, 1),
    PV(PS(0, kSlotOffsetType), static_cast<int>(ModuleId::Comp76Blue)),
    PV(PC(0, kCtlThreshold), 0.68), PV(PC(0, kCtlRatio), 0.75),
    PV(PC(0, kCtlAttack), 0.50), PV(PC(0, kCtlRelease), 0.20),
    PV(PC(0, kCtlMakeup), 0.45), PV(PC(0, kCtlMix), 0.35),
    PV(PC(0, kCtlScHpf), 0.05),
    PV(PS(1, kSlotOffsetType), static_cast<int>(ModuleId::Eq4K)),
    PV(PC(1, kCtlLow), 0.58), PV(PC(1, kCtlLowMid), 0.46),
    PV(PC(1, kCtlHighMid), 0.55), PV(PC(1, kCtlHigh), 0.55),
    PV(PC(1, kCtlHpf), 0.10), PV(PC(1, kCtlLpf), 1.00),
    PV(PS(2, kSlotOffsetType), static_cast<int>(ModuleId::Cleanup)),
    PV(PC(2, 0), 0.30), PV(PC(2, 1), 0.20), PV(PC(2, 2), 0.00),
    PV(PC(2, 3), 0.50),
    PV(PS(3, kSlotOffsetType), 0), PV(PS(4, kSlotOffsetType), 0),
    PV(PS(5, kSlotOffsetType), 0),
};

#undef PC
#undef PS
#undef PV

inline constexpr Preset kPresets[] = {
    {"Init", "Init", kInit, static_cast<int>(sizeof(kInit) / sizeof(kInit[0]))},
    {"Vocal", "Vocal Chain", kVocal, static_cast<int>(sizeof(kVocal) / sizeof(kVocal[0]))},
    {"Drum", "Drum Chain", kDrum, static_cast<int>(sizeof(kDrum) / sizeof(kDrum[0]))},
    {"Mix Bus", "Mix Bus Glue", kMixBusGlue, static_cast<int>(sizeof(kMixBusGlue) / sizeof(kMixBusGlue[0]))},
    {"Bass", "Bass DI", kBassDi, static_cast<int>(sizeof(kBassDi) / sizeof(kBassDi[0]))},
    {"Vocal", "Vocal Air", kVocalAir, static_cast<int>(sizeof(kVocalAir) / sizeof(kVocalAir[0]))},
    {"Drums", "Drum Room", kDrumRoom, static_cast<int>(sizeof(kDrumRoom) / sizeof(kDrumRoom[0]))},
    {"Mastering", "Mastering Chain", kMasteringChain, static_cast<int>(sizeof(kMasteringChain) / sizeof(kMasteringChain[0]))},
    {"Vocal", "Space Vocal", kSpaceVocal, static_cast<int>(sizeof(kSpaceVocal) / sizeof(kSpaceVocal[0]))},
    {"Guitar", "Guitar Spring", kGuitarSpring, static_cast<int>(sizeof(kGuitarSpring) / sizeof(kGuitarSpring[0]))},
    {"Parallel", "Parallel Smash", kParallelSmash, static_cast<int>(sizeof(kParallelSmash) / sizeof(kParallelSmash[0]))},
};

inline constexpr int kPresetCount = static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

} // namespace rack
