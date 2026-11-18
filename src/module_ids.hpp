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
  Filter,
  Motion,
  Sub,
  Double,
  DynamicEq,
  Multiband,
  Rider,
  Cleanup,
  SpacePlate,
  SpaceRoom,
  SpaceHall,
  SpaceSpring,
  SpaceDigital,
  SpaceTape,
  SpaceAnalog,
  SpacePingPong,
  SpaceChorus,
  SpaceFlanger,
  SpacePhaser,
  SpaceDimension,
  HearthComp,
  Glue,
  CompXpress,
  CompDigital,
  CompMolot,
  CompRed,
  Reel,
  Ember,
  Tame,
  Eq510,
  Pultec,
  JcClassicLimiter,
  JcTapeHead,
  JcTubeDriver,
  JcDaTube,
  JcFattener,
  JcAC1,
  JcLMC1,
  JcModernLimiter,
  Count
};

constexpr int kModuleCount = static_cast<int>(ModuleId::Count);

inline constexpr const char* kModuleLabels[] = {
    "Off",       "4K EQ",    "Hearth EQ", "Tilt",      "Air",
    "76 Blue",   "76 Black", "2A",        "3A",        "VCA",
    "Valve",     "Console",  "Bus",       "Tape",      "Transformer",
    "Gate",      "Transient", "Clip",     "De-Ess",   "Density",
    "Bleed Out", "Peak Limiter", "Image", "Crush", "Filter", "Motion",
    "Sub",       "Double",   "Dynamic EQ", "Multiband",
    "Rider",     "Cleanup",  "Plate",    "Room",      "Hall",
    "Spring",    "Digital",  "Tape",     "Analog",    "Ping-Pong",
    "Chorus",    "Flanger",  "Phaser",   "Dimension", "Hearth Comp",
    "Glue",      "Xpress",   "Digital",  "Molot",    "Red",
    "Reel",      "Ember",    "Tame",     "510 EQ",   "Pultec",
    "Classic Limiter", "Tape Head", "Tube Driver", "DaTube", "Fattener",
    "AC-1",      "LMC-1",    "Modern Limiter",
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
         id == static_cast<int>(ModuleId::CompVCA) ||
         id == static_cast<int>(ModuleId::HearthComp) ||
         id == static_cast<int>(ModuleId::Glue) ||
         id == static_cast<int>(ModuleId::CompXpress) ||
         id == static_cast<int>(ModuleId::CompDigital) ||
         id == static_cast<int>(ModuleId::CompMolot) ||
         id == static_cast<int>(ModuleId::CompRed);
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
         id == static_cast<int>(ModuleId::Limiter) ||
         id == static_cast<int>(ModuleId::Rider) ||
         id == static_cast<int>(ModuleId::Ember) ||
         id == static_cast<int>(ModuleId::JcClassicLimiter) ||
         id == static_cast<int>(ModuleId::JcAC1) ||
         id == static_cast<int>(ModuleId::JcLMC1) ||
         id == static_cast<int>(ModuleId::JcModernLimiter);
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
    case ModuleId::Filter:
      switch (control) {
        case 0: return 0.50;  // Cutoff
        case 1: return 0.20;  // Resonance
        case 2: return 0.50;  // Envelope
        case 3: return 0.50;  // Rate
        case 4: return 0.00;  // Mode: LP
        default: return 0.50;
      }
    case ModuleId::Motion:
      switch (control) {
        case 0: return 0.50;  // Rate
        case 1: return 0.50;  // Depth
        case 2: return 1.00;  // Width
        case 3: return 0.00;  // Mode: Tremolo
        case 4: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Sub:
      switch (control) {
        case 0: return 0.30;  // Frequency
        case 1: return 0.50;  // Amount
        case 2: return 0.20;  // Drive
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Double:
      switch (control) {
        case 0: return 0.30;  // Delay
        case 1: return 0.40;  // Detune
        case 2: return 0.50;  // Width
        case 3: return 0.35;  // Mix
        default: return 0.50;
      }
    case ModuleId::DynamicEq:
      switch (control) {
        case 0: return 0.50;  // Frequency
        case 1: return 0.50;  // Q
        case 2: return 0.60;  // Threshold
        case 3: return 0.50;  // Range
        case 4: return 0.50;  // Ratio
        case 5: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Multiband:
      switch (control) {
        case 0: return 0.50;  // Low threshold
        case 1: return 0.50;  // Mid threshold
        case 2: return 0.50;  // High threshold
        case 3: return 0.30;  // Ratio
        case 4: return 0.20;  // Attack
        case 5: return 0.40;  // Release
        case 6: return 0.00;  // Makeup
        default: return 0.50;
      }
    case ModuleId::Rider:
      switch (control) {
        case 0: return 0.60;  // Target
        case 1: return 0.50;  // Speed
        case 2: return 0.50;  // Range
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Cleanup:
      switch (control) {
        case 0: return 0.50;  // De-click
        case 1: return 0.50;  // De-hum
        case 2: return 0.00;  // Mains 50
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::SpacePlate:
    case ModuleId::SpaceRoom:
    case ModuleId::SpaceHall:
    case ModuleId::SpaceSpring:
      switch (control) {
        case 0: return 0.50;  // Size
        case 1: return 0.18;  // Decay
        case 2: return 0.12;  // Pre-delay
        case 3: return 0.50;  // Tone
        case 4: return 0.70;  // Width
        case 5: return 0.30;  // Mix
        default: return 0.50;
      }
    case ModuleId::SpaceDigital:
    case ModuleId::SpaceTape:
    case ModuleId::SpaceAnalog:
    case ModuleId::SpacePingPong:
      switch (control) {
        case 0: return 0.19;  // Time
        case 1: return 0.35;  // Feedback
        case 2: return 0.50;  // Tone
        case 3: return 0.15;  // Mod
        case 4: return 0.50;  // Width
        case 5: return 0.25;  // Mix
        default: return 0.50;
      }
    case ModuleId::SpaceChorus:
    case ModuleId::SpaceFlanger:
    case ModuleId::SpacePhaser:
    case ModuleId::SpaceDimension:
      switch (control) {
        case 0: return 0.25;  // Rate
        case 1: return 0.50;  // Depth
        case 2: return 0.20;  // Feedback
        case 3: return 0.70;  // Width
        case 4: return 0.40;  // Mix
        default: return 0.50;
      }
    case ModuleId::HearthComp:
      switch (control) {
        case kCtlThreshold: return 0.50;
        case kCtlRatio: return 0.30;
        case kCtlAttack: return 0.40;
        case kCtlRelease: return 0.50;
        case kCtlMakeup: return 0.50;
        case kCtlMix: return 1.00;
        case kCtlScHpf: return 0.10;
        default: return 0.50;
      }
    case ModuleId::Glue:
      switch (control) {
        case kCtlThreshold: return 0.50;
        case kCtlRatio: return 0.20;
        case kCtlAttack: return 0.50;
        case kCtlRelease: return 0.50;
        case kCtlMakeup: return 0.50;
        case kCtlMix: return 1.00;
        case kCtlScHpf: return 0.10;
        default: return 0.50;
      }
    case ModuleId::CompXpress:
      switch (control) {
        case kCtlThreshold: return 0.50;  // Input
        case kCtlRatio: return 0.50;
        case kCtlAttack: return 0.30;
        case kCtlRelease: return 0.40;
        case kCtlMakeup: return 0.50;     // Output
        case kCtlMix: return 1.00;
        case kCtlScHpf: return 0.10;
        case 7: return 0.00;              // Distortion off
        default: return 0.50;
      }
    case ModuleId::CompDigital:
      switch (control) {
        case kCtlThreshold: return 0.50;
        case kCtlRatio: return 0.50;
        case kCtlAttack: return 0.30;
        case kCtlRelease: return 0.40;
        case kCtlMakeup: return 0.50;
        case kCtlMix: return 1.00;
        case kCtlScHpf: return 0.10;
        default: return 0.50;
      }
    case ModuleId::CompMolot:
      switch (control) {
        case kCtlThreshold: return 0.50;
        case kCtlRatio: return 0.50;
        case kCtlAttack: return 0.30;
        case kCtlRelease: return 0.40;
        case kCtlMakeup: return 0.50;
        case kCtlMix: return 1.00;
        case kCtlScHpf: return 0.10;
        default: return 0.50;
      }
    case ModuleId::CompRed:
      switch (control) {
        case kCtlThreshold: return 0.50;
        case kCtlRatio: return 0.50;
        case kCtlAttack: return 0.30;
        case kCtlRelease: return 0.40;
        case kCtlMakeup: return 0.50;
        case kCtlMix: return 1.00;
        case kCtlScHpf: return 0.10;
        default: return 0.50;
      }
    case ModuleId::Reel:
      switch (control) {
        case 0: return 0.00;  // Reel
        case 1: return 0.20;  // Drive
        case 2: return 0.10;  // Wow
        case 3: return 0.25;  // Mix
        default: return 0.50;
      }
    case ModuleId::Ember:
      return control == 0 ? 0.00 : 0.50;
    case ModuleId::Tame:
      switch (control) {
        case 0: return 0.00;  // Amount
        case 1: return 0.50;  // Focus
        case 2: return 0.50;  // Selectivity
        case 3: return 0.50;  // Sensitivity
        case 4: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::Eq510:
      switch (control) {
        case 0: return 0.50;  // Low
        case 1: return 0.50;  // Mid
        case 2: return 0.50;  // High
        case 3: return 0.00;  // HPF
        default: return 0.50;
      }
    case ModuleId::Pultec:
      switch (control) {
        case 0: return 0.00;  // Low boost
        case 1: return 0.00;  // Low atten
        case 2: return 0.00;  // High boost
        case 3: return 0.00;  // High atten
        case 4: return 0.50;  // Bandwidth
        default: return 0.50;
      }
    case ModuleId::JcClassicLimiter:
      switch (control) {
        case 0: return 0.75;  // Threshold
        case 1: return 0.50;  // Output
        case 2: return 1.00;  // Mix
        default: return 0.50;
      }
    case ModuleId::JcTapeHead:
      switch (control) {
        case 0: return 0.28;  // Drive
        case 1: return 0.86;  // Trim
        case 2: return 0.00;  // Normal
        case 3: return 1.00;  // Clip on
        default: return 0.50;
      }
    case ModuleId::JcTubeDriver:
      switch (control) {
        case 0: return 0.50;  // Input
        case 1: return 0.20;  // Drive
        case 2: return 0.50;  // Bias
        case 3: return 0.00;  // Boost off
        case 4: return 0.50;  // Output
        default: return 0.50;
      }
    case ModuleId::JcDaTube:
      switch (control) {
        case 0: return 0.30;  // Drive
        case 1: return 1.00;  // Balance
        case 2: return 0.75;  // Output
        case 3: return 0.00;  // Bit reduction off
        case 4: return 0.00;  // Fold back off
        default: return 0.50;
      }
    case ModuleId::JcFattener:
      switch (control) {
        case 0: return 0.00;  // Fattness
        case 1: return 0.00;  // Color
        case 2: return 0.50;  // Gain
        default: return 0.50;
      }
    case ModuleId::JcAC1:
      switch (control) {
        case 0: return 0.50;  // Input
        case 1: return 0.50;  // Output
        case 2: return 0.50;  // Drive
        case 3: return 0.11;  // Comp
        case 4: return 0.04;  // Attack
        case 5: return 0.57;  // Release
        case 6: return 0.00;  // AC1 mode
        default: return 0.50;
      }
    case ModuleId::JcLMC1:
      switch (control) {
        case 0: return 0.50;  // Input
        case 1: return 0.50;  // Threshold
        case 2: return 0.50;  // Output
        case 3: return 1.00;  // Box Tone on
        default: return 0.50;
      }
    case ModuleId::JcModernLimiter:
      switch (control) {
        case 0: return 0.50;  // Input
        case 1: return 0.95;  // Threshold
        case 2: return 0.50;  // Output
        case 3: return 1.00;  // Mix
        default: return 0.50;
      }
    default:
      return 0.50;
  }
}

} // namespace rack
