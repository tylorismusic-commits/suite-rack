// Suite Rack — module identifiers and shared control defaults
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace rack {

// Appended-only. The numeric value is stored in host automation and in rack
// state, so never reorder existing entries.
enum class ModuleId : int {
  Off = 0,
  Eq4K,
  EqHearth,
  EqTilt,
  EqAir,
  Comp76Blue,
  Comp76Black,
  Comp2A,
  Comp3A,
  CompVCA,
  ColourValve,
  ColourConsole,
  ColourBus,
  ColourTape,
  ColourTransformer,
  DynGate,
  DynTransient,
  DynClip,
  DynDeEss,
  Density,
  BleedOut,
  Limiter,
  Image,
  Crush,
  Count
};

constexpr int kModuleCount = static_cast<int>(ModuleId::Count);

inline constexpr const char* kModuleLabels[] = {
    "Off",       "4K EQ",    "Hearth EQ", "Tilt",      "Air",
    "76 Blue",   "76 Black", "2A",        "3A",        "VCA",
    "Valve",     "Console",  "Bus",       "Tape",      "Transformer",
    "Gate",      "Transient", "Clip",     "De-Ess",   "Density",
    "Bleed Out", "Peak Limiter", "Image", "Crush",
};

static_assert(sizeof(kModuleLabels) / sizeof(kModuleLabels[0]) == kModuleCount,
              "kModuleLabels must cover every module id");

constexpr int kRackControlCount = 12;

// Shared control indices for the first modules. Later modules can use any of
// the twelve slots; the module descriptor says which ones are visible.
constexpr int kCtlLow = 0;
constexpr int kCtlLowMid = 1;
constexpr int kCtlHighMid = 2;
constexpr int kCtlHigh = 3;
constexpr int kCtlHpf = 4;
constexpr int kCtlLpf = 5;

constexpr int kCtlThreshold = 0;
constexpr int kCtlRatio = 1;
constexpr int kCtlAttack = 2;
constexpr int kCtlRelease = 3;
constexpr int kCtlMakeup = 4;
constexpr int kCtlMix = 5;
constexpr int kCtlScHpf = 6;

inline constexpr bool validModule(int id) noexcept {
  return id >= 0 && id < kModuleCount;
}

inline constexpr bool moduleIsEq(int id) noexcept {
  return id == static_cast<int>(ModuleId::Eq4K) ||
         id == static_cast<int>(ModuleId::EqHearth) ||
         id == static_cast<int>(ModuleId::EqTilt) ||
         id == static_cast<int>(ModuleId::EqAir);
}

inline constexpr bool moduleIsComp(int id) noexcept {
  return id == static_cast<int>(ModuleId::Comp76Blue) ||
         id == static_cast<int>(ModuleId::Comp76Black) ||
         id == static_cast<int>(ModuleId::Comp2A) ||
         id == static_cast<int>(ModuleId::Comp3A) ||
         id == static_cast<int>(ModuleId::CompVCA);
}

inline constexpr bool moduleIsColour(int id) noexcept {
  return id == static_cast<int>(ModuleId::ColourValve) ||
         id == static_cast<int>(ModuleId::ColourConsole) ||
         id == static_cast<int>(ModuleId::ColourBus) ||
         id == static_cast<int>(ModuleId::ColourTape) ||
         id == static_cast<int>(ModuleId::ColourTransformer);
}

inline constexpr bool moduleIsDynamics(int id) noexcept {
  return id == static_cast<int>(ModuleId::DynGate) ||
         id == static_cast<int>(ModuleId::DynTransient) ||
         id == static_cast<int>(ModuleId::DynClip) ||
         id == static_cast<int>(ModuleId::DynDeEss) ||
         id == static_cast<int>(ModuleId::BleedOut) ||
         id == static_cast<int>(ModuleId::Limiter);
}

inline constexpr bool moduleHasGr(int id) noexcept {
  return moduleIsComp(id) || id == static_cast<int>(ModuleId::DynGate) ||
         id == static_cast<int>(ModuleId::DynDeEss) ||
         id == static_cast<int>(ModuleId::BleedOut) ||
         id == static_cast<int>(ModuleId::Limiter);
}

// All controls are normalised to 0..1. The module descriptor maps them to
// labels, formats and DSP behaviour. Having one source of truth here keeps the
// host parameter defaults and the module defaults in step.
constexpr double defaultControlValue(ModuleId id, int control) noexcept {
  switch (id) {
    case ModuleId::Eq4K:
      switch (control) {
        case kCtlLow: return 0.60;
        case kCtlLowMid: return 0.45;
        case kCtlHighMid: return 0.55;
        case kCtlHigh: return 0.57;
        case kCtlHpf: return 0.10;
        case kCtlLpf: return 1.00;
        default: return 0.50;
      }
    case ModuleId::EqHearth:
      switch (control) {
        case kCtlLow: return 0.55;
        case kCtlLowMid: return 0.48;
        case kCtlHighMid: return 0.52;
        case kCtlHigh: return 0.55;
        case kCtlHpf: return 0.08;
        case kCtlLpf: return 1.00;
        default: return 0.50;
      }
    case ModuleId::EqTilt:
      switch (control) {
        case 0: return 0.50;
        case 1: return 0.05;
        case 2: return 1.00;
        default: return 0.50;
      }
    case ModuleId::EqAir:
      switch (control) {
        case 0: return 0.60;
        case 1: return 0.60;
        case 2: return 0.05;
        default: return 0.50;
      }
    case ModuleId::Comp76Blue:
    case ModuleId::Comp76Black:
      switch (control) {
        case kCtlThreshold: return 0.50;
        case kCtlRatio: return 0.50;
        case kCtlAttack: return 0.25;
        case kCtlRelease: return 0.35;
        case kCtlMakeup: return 0.80;
        case kCtlMix: return 1.00;
        case kCtlScHpf: return 0.05;
        default: return 0.50;
      }
    case ModuleId::Comp2A:
    case ModuleId::Comp3A:
      switch (control) {
        case 0: return 0.50;
        case kCtlMakeup: return 0.50;
        case kCtlMix: return 1.00;
        default: return 0.50;
      }
    case ModuleId::CompVCA:
      switch (control) {
        case kCtlThreshold: return 0.50;
        case kCtlRatio: return 0.50;
        case kCtlAttack: return 0.25;
        case kCtlRelease: return 0.35;
        case kCtlMakeup: return 0.50;
        case kCtlMix: return 1.00;
        case kCtlScHpf: return 0.05;
        default: return 0.50;
      }
    case ModuleId::ColourValve:
    case ModuleId::ColourConsole:
    case ModuleId::ColourBus:
    case ModuleId::ColourTape:
    case ModuleId::ColourTransformer:
      switch (control) {
        case 0: return 0.45;  // Drive
        case 1: return 0.50;  // Character
        case 2: return 0.50;  // Tone
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::DynGate:
      switch (control) {
        case 0: return 0.50;  // Threshold
        case 1: return 0.50;  // Range
        case 2: return 0.15;  // Attack
        case 3: return 0.35;  // Release
        case 4: return 0.20;  // Hold
        default: return 0.50;
      }
    case ModuleId::DynTransient:
      switch (control) {
        case 0: return 0.50;  // Attack
        case 1: return 0.50;  // Sustain
        case 2: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::DynClip:
      switch (control) {
        case 0: return 0.45;  // Drive
        case 1: return 0.75;  // Ceiling
        case 2: return 0.00;  // Mode: Soft
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::DynDeEss:
      switch (control) {
        case 0: return 0.60;  // Frequency
        case 1: return 0.60;  // Threshold
        case 2: return 0.50;  // Range
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Density:
      switch (control) {
        case 0: return 0.50;  // Input
        case 1: return 0.50;  // Density
        case 2: return 0.50;  // Output
        default: return 0.50;
      }
    case ModuleId::BleedOut:
      switch (control) {
        case 0: return 0.00;  // Profile: Kick
        case 1: return 0.50;  // Threshold
        case 2: return 0.70;  // DeBleed
        case 3: return 0.75;  // Ghost
        case 4: return 1.00;  // Ghost On
        default: return 0.50;
      }
    case ModuleId::Limiter:
      switch (control) {
        case 0: return 0.50;  // Input
        case 1: return 0.90;  // Ceiling
        case 2: return 0.50;  // Release
        case 3: return 1.00;  // Link
        default: return 0.50;
      }
    case ModuleId::Image:
      switch (control) {
        case 0: return 0.50;  // Width
        case 1: return 0.00;  // Mono
        case 2: return 0.50;  // Balance
        case 3: return 0.00;  // M/S off
        default: return 0.50;
      }
    case ModuleId::Crush:
      switch (control) {
        case 0: return 1.00;  // Bits: 16
        case 1: return 1.00;  // Rate: full
        case 2: return 1.00;  // Tone: open
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    default:
      return 0.50;
  }
}

} // namespace rack
