// Suite Rack — module registry
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Every browser entry is a compact 500-series module: one job, a handful of
// controls and no deep pages. The registry is the single place that maps a
// module id to its name, category, visible controls and DSP family.
#pragma once

#include <cstddef>

#include "module_ids.hpp"
#include "params.hpp"

namespace rack {

enum class ControlFormat {
  Percent,
  ThresholdDb,
  GainDb,
  Ratio,
  AttackMs,
  ReleaseMs,
  ScHpfHz,
  EqHpfHz,
  EqLpfHz,
  AirFreqHz,
  TiltDb,
  AirDb,
  GateThresholdDb,
  RangeDb,
  GateAttackMs,
  GateReleaseMs,
  HoldMs,
  BipolarPercent,
  ClipDriveDb,
  ClipCeilingDb,
  ClipMode,
  DeEssFreqHz,
  DeEssRangeDb,
  LimiterCeilingDb,
  LimiterReleaseMs,
  WidthPercent,
  MonoHz,
  CrushBits,
  CrushToneHz,
  FilterCutoffHz,
  LfoRateHz,
  FilterMode,
  MotionMode,
  SubFreqHz,
  DelayMs,
  DetuneMs,
  QValue,
  DynRangeDb,
  DynRatio,
  MakeupDb,
  RiderTargetDb,
  RiderSpeedMs,
  SpaceDecayS,
  SpacePreDelayMs,
  SpaceDelayMs,
};

struct ControlDesc {
  int param = 0;          // index into the slot's twelve generic controls
  const char* label = ""; // short faceplate label
  const char* name = "";  // host / tooltip name
  ControlFormat format = ControlFormat::Percent;
  bool stepped = false;
  Labels labels{};
  bool toggled = false;
};

struct ModuleDesc {
  ModuleId id = ModuleId::Off;
  const char* name = "Off";
  const char* category = "Off";
  const char* description = "Empty slot";
  bool isEq = false;
  bool isComp = false;
  int model = 0;          // DSP sub-model (compressor voicing)
  int controlCount = 0;
  const ControlDesc* controls = nullptr;
};

namespace detail {

inline constexpr ControlDesc kEq4KControls[] = {
    {kCtlLow, "LOW", "Low", ControlFormat::GainDb},
    {kCtlLowMid, "LOW MID", "Low Mid", ControlFormat::GainDb},
    {kCtlHighMid, "HIGH MID", "High Mid", ControlFormat::GainDb},
    {kCtlHigh, "HIGH", "High", ControlFormat::GainDb},
    {kCtlHpf, "HPF", "High-Pass", ControlFormat::EqHpfHz},
    {kCtlLpf, "LPF", "Low-Pass", ControlFormat::EqLpfHz},
};

inline constexpr ControlDesc kEqHearthControls[] = {
    {kCtlLow, "LOW", "Low", ControlFormat::GainDb},
    {kCtlLowMid, "LOW MID", "Low Mid", ControlFormat::GainDb},
    {kCtlHighMid, "HIGH MID", "High Mid", ControlFormat::GainDb},
    {kCtlHigh, "HIGH", "High", ControlFormat::GainDb},
    {kCtlHpf, "HPF", "High-Pass", ControlFormat::EqHpfHz},
    {kCtlLpf, "LPF", "Low-Pass", ControlFormat::EqLpfHz},
};

inline constexpr ControlDesc kEqTiltControls[] = {
    {0, "TILT", "Tilt", ControlFormat::TiltDb},
    {1, "HPF", "High-Pass", ControlFormat::EqHpfHz},
    {2, "LPF", "Low-Pass", ControlFormat::EqLpfHz},
};

inline constexpr ControlDesc kEqAirControls[] = {
    {0, "AIR", "Air", ControlFormat::AirDb},
    {1, "FREQ", "Frequency", ControlFormat::AirFreqHz},
    {2, "HPF", "High-Pass", ControlFormat::EqHpfHz},
};

inline constexpr ControlDesc kComp76Controls[] = {
    {kCtlThreshold, "THR", "Threshold", ControlFormat::ThresholdDb},
    {kCtlRatio, "RAT", "Ratio", ControlFormat::Ratio},
    {kCtlAttack, "ATK", "Attack", ControlFormat::AttackMs},
    {kCtlRelease, "REL", "Release", ControlFormat::ReleaseMs},
    {kCtlMakeup, "MAKE", "Makeup", ControlFormat::GainDb},
    {kCtlMix, "MIX", "Mix", ControlFormat::Percent},
    {kCtlScHpf, "SC", "Sidechain HPF", ControlFormat::ScHpfHz},
};

inline constexpr ControlDesc kCompOptoControls[] = {
    {0, "PEAK", "Peak Reduction", ControlFormat::Percent},
    {kCtlMakeup, "GAIN", "Gain", ControlFormat::GainDb},
    {kCtlMix, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kCompVcaControls[] = {
    {kCtlThreshold, "THR", "Threshold", ControlFormat::ThresholdDb},
    {kCtlRatio, "RAT", "Ratio", ControlFormat::Ratio},
    {kCtlAttack, "ATK", "Attack", ControlFormat::AttackMs},
    {kCtlRelease, "REL", "Release", ControlFormat::ReleaseMs},
    {kCtlMakeup, "MAKE", "Makeup", ControlFormat::GainDb},
    {kCtlMix, "MIX", "Mix", ControlFormat::Percent},
    {kCtlScHpf, "SC", "Sidechain HPF", ControlFormat::ScHpfHz},
};

inline constexpr ControlDesc kColourControls[] = {
    {0, "DRIVE", "Drive", ControlFormat::Percent},
    {1, "CHAR", "Character", ControlFormat::Percent},
    {2, "TONE", "Tone", ControlFormat::Percent},
    {3, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kGateControls[] = {
    {0, "THR", "Threshold", ControlFormat::GateThresholdDb},
    {1, "RANGE", "Range", ControlFormat::RangeDb},
    {2, "ATK", "Attack", ControlFormat::GateAttackMs},
    {3, "REL", "Release", ControlFormat::GateReleaseMs},
    {4, "HOLD", "Hold", ControlFormat::HoldMs},
};

inline constexpr ControlDesc kTransientControls[] = {
    {0, "ATTACK", "Attack", ControlFormat::BipolarPercent},
    {1, "SUSTAIN", "Sustain", ControlFormat::BipolarPercent},
    {2, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kClipControls[] = {
    {0, "DRIVE", "Drive", ControlFormat::ClipDriveDb},
    {1, "CEIL", "Ceiling", ControlFormat::ClipCeilingDb},
    {2, "MODE", "Mode", ControlFormat::ClipMode},
    {3, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr const char* kBleedProfiles[] = {
    "Kick", "Snare", "Tom", "Hat", "Ride", "Room", "OH", "Bus",
};

inline constexpr ControlDesc kBleedOutControls[] = {
    {0, "PROFILE", "Profile", ControlFormat::Percent, true,
     labels(kBleedProfiles)},
    {1, "THR", "Threshold", ControlFormat::GateThresholdDb},
    {2, "DEBLEED", "DeBleed", ControlFormat::Percent},
    {3, "GHOST", "Ghost", ControlFormat::Percent},
    {4, "GHOST ON", "Ghost On", ControlFormat::Percent, false, {}, true},
};

inline constexpr const char* kFilterModeLabels[] = {"LP", "BP", "HP"};
inline constexpr const char* kMotionModeLabels[] = {"Tremolo", "Auto-Pan",
                                                    "Rotary"};

inline constexpr const char* kMainsLabels[] = {"50 Hz", "60 Hz"};

inline constexpr ControlDesc kSpaceReverbControls[] = {
    {0, "SIZE", "Size", ControlFormat::Percent},
    {1, "DECAY", "Decay", ControlFormat::SpaceDecayS},
    {2, "PRE", "Pre-Delay", ControlFormat::SpacePreDelayMs},
    {3, "TONE", "Tone", ControlFormat::Percent},
    {4, "WIDTH", "Width", ControlFormat::Percent},
    {5, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kSpaceDelayControls[] = {
    {0, "TIME", "Time", ControlFormat::SpaceDelayMs},
    {1, "FB", "Feedback", ControlFormat::Percent},
    {2, "TONE", "Tone", ControlFormat::Percent},
    {3, "MOD", "Mod", ControlFormat::Percent},
    {4, "WIDTH", "Width", ControlFormat::Percent},
    {5, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kSpaceModControls[] = {
    {0, "RATE", "Rate", ControlFormat::LfoRateHz},
    {1, "DEPTH", "Depth", ControlFormat::Percent},
    {2, "FB", "Feedback", ControlFormat::Percent},
    {3, "WIDTH", "Width", ControlFormat::Percent},
    {4, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kRiderControls[] = {
    {0, "TARGET", "Target", ControlFormat::RiderTargetDb},
    {1, "SPEED", "Speed", ControlFormat::RiderSpeedMs},
    {2, "RANGE", "Range", ControlFormat::DynRangeDb},
    {3, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kCleanupControls[] = {
    {0, "DECLICK", "De-Click", ControlFormat::Percent},
    {1, "DEHUM", "De-Hum", ControlFormat::Percent},
    {2, "MAINS", "Mains", ControlFormat::Percent, true, labels(kMainsLabels)},
    {3, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kDynamicEqControls[] = {
    {0, "FREQ", "Frequency", ControlFormat::FilterCutoffHz},
    {1, "Q", "Q", ControlFormat::QValue},
    {2, "THR", "Threshold", ControlFormat::ThresholdDb},
    {3, "RANGE", "Range", ControlFormat::DynRangeDb},
    {4, "RATIO", "Ratio", ControlFormat::DynRatio},
    {5, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kMultibandControls[] = {
    {0, "LOW", "Low Threshold", ControlFormat::GateThresholdDb},
    {1, "MID", "Mid Threshold", ControlFormat::GateThresholdDb},
    {2, "HIGH", "High Threshold", ControlFormat::GateThresholdDb},
    {3, "RATIO", "Ratio", ControlFormat::DynRatio},
    {4, "ATK", "Attack", ControlFormat::GateAttackMs},
    {5, "REL", "Release", ControlFormat::GateReleaseMs},
    {6, "MAKE", "Makeup", ControlFormat::MakeupDb},
};

inline constexpr ControlDesc kSubControls[] = {
    {0, "FREQ", "Frequency", ControlFormat::SubFreqHz},
    {1, "AMOUNT", "Amount", ControlFormat::Percent},
    {2, "DRIVE", "Drive", ControlFormat::Percent},
    {3, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kDoubleControls[] = {
    {0, "DELAY", "Delay", ControlFormat::DelayMs},
    {1, "DETUNE", "Detune", ControlFormat::DetuneMs},
    {2, "WIDTH", "Width", ControlFormat::Percent},
    {3, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kFilterControls[] = {
    {0, "CUTOFF", "Cutoff", ControlFormat::FilterCutoffHz},
    {1, "RES", "Resonance", ControlFormat::Percent},
    {2, "ENV", "Envelope", ControlFormat::BipolarPercent},
    {3, "RATE", "Rate", ControlFormat::LfoRateHz},
    {4, "MODE", "Mode", ControlFormat::FilterMode, true,
     labels(kFilterModeLabels)},
};

inline constexpr ControlDesc kMotionControls[] = {
    {0, "RATE", "Rate", ControlFormat::LfoRateHz},
    {1, "DEPTH", "Depth", ControlFormat::Percent},
    {2, "WIDTH", "Width", ControlFormat::Percent},
    {3, "MODE", "Mode", ControlFormat::MotionMode, true,
     labels(kMotionModeLabels)},
    {4, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kImageControls[] = {
    {0, "WIDTH", "Width", ControlFormat::WidthPercent},
    {1, "MONO", "Mono Maker", ControlFormat::MonoHz},
    {2, "BAL", "Balance", ControlFormat::BipolarPercent},
    {3, "M/S", "M/S", ControlFormat::Percent, false, {}, true},
};

inline constexpr ControlDesc kCrushControls[] = {
    {0, "BITS", "Bits", ControlFormat::CrushBits},
    {1, "RATE", "Rate", ControlFormat::Percent},
    {2, "TONE", "Tone", ControlFormat::CrushToneHz},
    {3, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ControlDesc kLimiterControls[] = {
    {0, "INPUT", "Input", ControlFormat::TiltDb},
    {1, "CEIL", "Ceiling", ControlFormat::LimiterCeilingDb},
    {2, "REL", "Release", ControlFormat::LimiterReleaseMs},
    {3, "LINK", "Link", ControlFormat::Percent},
};

inline constexpr ControlDesc kDensityControls[] = {
    {0, "INPUT", "Input", ControlFormat::TiltDb},
    {1, "DENSITY", "Density", ControlFormat::Percent},
    {2, "OUTPUT", "Output", ControlFormat::TiltDb},
};

inline constexpr ControlDesc kDeEssControls[] = {
    {0, "FREQ", "Frequency", ControlFormat::DeEssFreqHz},
    {1, "THR", "Threshold", ControlFormat::ThresholdDb},
    {2, "RANGE", "Range", ControlFormat::DeEssRangeDb},
    {3, "MIX", "Mix", ControlFormat::Percent},
};

inline constexpr ModuleDesc kModules[] = {
    {ModuleId::Off, "Off", "Off", "Empty slot", false, false, 0, 0, nullptr},
    {ModuleId::Eq4K, "4K EQ", "EQ", "Four-band console EQ", true, false, 0,
     static_cast<int>(sizeof(kEq4KControls) / sizeof(kEq4KControls[0])),
     kEq4KControls},
    {ModuleId::EqHearth, "Hearth EQ", "EQ", "Three-band inductor console EQ",
     true, false, 0,
     static_cast<int>(sizeof(kEqHearthControls) / sizeof(kEqHearthControls[0])),
     kEqHearthControls},
    {ModuleId::EqTilt, "Tilt", "EQ", "Broad tilt EQ with filters", true, false,
     0, static_cast<int>(sizeof(kEqTiltControls) / sizeof(kEqTiltControls[0])),
     kEqTiltControls},
    {ModuleId::EqAir, "Air", "EQ", "High-band air lift", true, false, 0,
     static_cast<int>(sizeof(kEqAirControls) / sizeof(kEqAirControls[0])),
     kEqAirControls},
    {ModuleId::Comp76Blue, "76 Blue", "Comp", "FET compressor, blue voicing",
     false, true, 0,
     static_cast<int>(sizeof(kComp76Controls) / sizeof(kComp76Controls[0])),
     kComp76Controls},
    {ModuleId::Comp76Black, "76 Black", "Comp",
     "FET compressor, black voicing", false, true, 1,
     static_cast<int>(sizeof(kComp76Controls) / sizeof(kComp76Controls[0])),
     kComp76Controls},
    {ModuleId::Comp2A, "2A", "Comp", "Optical leveler", false, true, 2,
     static_cast<int>(sizeof(kCompOptoControls) /
                      sizeof(kCompOptoControls[0])),
     kCompOptoControls},
    {ModuleId::Comp3A, "3A", "Comp", "Faster optical leveler", false, true, 3,
     static_cast<int>(sizeof(kCompOptoControls) /
                      sizeof(kCompOptoControls[0])),
     kCompOptoControls},
    {ModuleId::CompVCA, "VCA", "Comp", "Stereo bus VCA compressor", false, true,
     4, static_cast<int>(sizeof(kCompVcaControls) /
                         sizeof(kCompVcaControls[0])),
     kCompVcaControls},
    {ModuleId::ColourValve, "Valve", "Colour", "Valve saturation", false, false,
     0, static_cast<int>(sizeof(kColourControls) / sizeof(kColourControls[0])),
     kColourControls},
    {ModuleId::ColourConsole, "Console", "Colour", "Console saturation", false,
     false, 0,
     static_cast<int>(sizeof(kColourControls) / sizeof(kColourControls[0])),
     kColourControls},
    {ModuleId::ColourBus, "Bus", "Colour", "Bus saturation", false, false, 0,
     static_cast<int>(sizeof(kColourControls) / sizeof(kColourControls[0])),
     kColourControls},
    {ModuleId::ColourTape, "Tape", "Colour", "Tape saturation", false, false, 0,
     static_cast<int>(sizeof(kColourControls) / sizeof(kColourControls[0])),
     kColourControls},
    {ModuleId::ColourTransformer, "Transformer", "Colour",
     "Transformer saturation", false, false, 0,
     static_cast<int>(sizeof(kColourControls) / sizeof(kColourControls[0])),
     kColourControls},
    {ModuleId::DynGate, "Gate", "Dyn", "Expander / gate", false, false, 0,
     static_cast<int>(sizeof(kGateControls) / sizeof(kGateControls[0])),
     kGateControls},
    {ModuleId::DynTransient, "Transient", "Dyn", "Transient shaper", false,
     false, 0,
     static_cast<int>(sizeof(kTransientControls) /
                      sizeof(kTransientControls[0])),
     kTransientControls},
    {ModuleId::DynClip, "Clip", "Dyn", "Soft / hard / diode clipper", false,
     false, 0,
     static_cast<int>(sizeof(kClipControls) / sizeof(kClipControls[0])),
     kClipControls},
    {ModuleId::DynDeEss, "De-Ess", "Dyn", "High-band de-esser", false, false,
     0, static_cast<int>(sizeof(kDeEssControls) / sizeof(kDeEssControls[0])),
     kDeEssControls},
    {ModuleId::Density, "Density", "Colour", "Harmonic density", false, false,
     0, static_cast<int>(sizeof(kDensityControls) /
                         sizeof(kDensityControls[0])),
     kDensityControls},
    {ModuleId::BleedOut, "Bleed Out", "Dyn", "Multiband de-bleed gate", false,
     false, 0,
     static_cast<int>(sizeof(kBleedOutControls) /
                      sizeof(kBleedOutControls[0])),
     kBleedOutControls},
    {ModuleId::Limiter, "Peak Limiter", "Dyn", "Lookahead true-peak limiter",
     false, false, 0,
     static_cast<int>(sizeof(kLimiterControls) /
                      sizeof(kLimiterControls[0])),
     kLimiterControls},
    {ModuleId::Image, "Image", "Space", "Width, mono-maker, balance and M/S",
     false, false, 0,
     static_cast<int>(sizeof(kImageControls) / sizeof(kImageControls[0])),
     kImageControls},
    {ModuleId::Crush, "Crush", "Colour", "Bitcrusher and sample-rate reducer",
     false, false, 0,
     static_cast<int>(sizeof(kCrushControls) / sizeof(kCrushControls[0])),
     kCrushControls},
    {ModuleId::Filter, "Filter", "Dyn", "Envelope / LFO auto-filter", false,
     false, 0,
     static_cast<int>(sizeof(kFilterControls) / sizeof(kFilterControls[0])),
     kFilterControls},
    {ModuleId::Motion, "Motion", "Space", "Tremolo, auto-pan and rotary", false,
     false, 0,
     static_cast<int>(sizeof(kMotionControls) / sizeof(kMotionControls[0])),
     kMotionControls},
    {ModuleId::Sub, "Sub", "Dyn", "Subharmonic synthesizer", false, false, 0,
     static_cast<int>(sizeof(kSubControls) / sizeof(kSubControls[0])),
     kSubControls},
    {ModuleId::Double, "Double", "Space", "Doubler / microshift", false, false, 0,
     static_cast<int>(sizeof(kDoubleControls) / sizeof(kDoubleControls[0])),
     kDoubleControls},
    {ModuleId::DynamicEq, "Dynamic EQ", "Dyn", "Single-band dynamic EQ", false,
     false, 0,
     static_cast<int>(sizeof(kDynamicEqControls) /
                      sizeof(kDynamicEqControls[0])),
     kDynamicEqControls},
    {ModuleId::Multiband, "Multiband", "Dyn", "Three-band compressor", false,
     false, 0,
     static_cast<int>(sizeof(kMultibandControls) /
                      sizeof(kMultibandControls[0])),
     kMultibandControls},
    {ModuleId::Rider, "Rider", "Dyn", "Auto-level / vocal rider", false, false,
     0, static_cast<int>(sizeof(kRiderControls) / sizeof(kRiderControls[0])),
     kRiderControls},
    {ModuleId::Cleanup, "Cleanup", "Dyn", "De-click and de-hum", false, false,
     0,
     static_cast<int>(sizeof(kCleanupControls) / sizeof(kCleanupControls[0])),
     kCleanupControls},
    {ModuleId::SpacePlate, "Plate", "Space", "Plate reverb", false, false, 0,
     static_cast<int>(sizeof(kSpaceReverbControls) /
                      sizeof(kSpaceReverbControls[0])),
     kSpaceReverbControls},
    {ModuleId::SpaceRoom, "Room", "Space", "Room reverb", false, false, 0,
     static_cast<int>(sizeof(kSpaceReverbControls) /
                      sizeof(kSpaceReverbControls[0])),
     kSpaceReverbControls},
    {ModuleId::SpaceHall, "Hall", "Space", "Hall reverb", false, false, 0,
     static_cast<int>(sizeof(kSpaceReverbControls) /
                      sizeof(kSpaceReverbControls[0])),
     kSpaceReverbControls},
    {ModuleId::SpaceSpring, "Spring", "Space", "Spring reverb", false, false, 0,
     static_cast<int>(sizeof(kSpaceReverbControls) /
                      sizeof(kSpaceReverbControls[0])),
     kSpaceReverbControls},
    {ModuleId::SpaceDigital, "Digital", "Space", "Digital delay", false, false,
     0,
     static_cast<int>(sizeof(kSpaceDelayControls) /
                      sizeof(kSpaceDelayControls[0])),
     kSpaceDelayControls},
    {ModuleId::SpaceTape, "Tape", "Space", "Tape delay", false, false, 0,
     static_cast<int>(sizeof(kSpaceDelayControls) /
                      sizeof(kSpaceDelayControls[0])),
     kSpaceDelayControls},
    {ModuleId::SpaceAnalog, "Analog", "Space", "Analog delay", false, false, 0,
     static_cast<int>(sizeof(kSpaceDelayControls) /
                      sizeof(kSpaceDelayControls[0])),
     kSpaceDelayControls},
    {ModuleId::SpacePingPong, "Ping-Pong", "Space", "Ping-pong delay", false,
     false, 0,
     static_cast<int>(sizeof(kSpaceDelayControls) /
                      sizeof(kSpaceDelayControls[0])),
     kSpaceDelayControls},
    {ModuleId::SpaceChorus, "Chorus", "Space", "Chorus", false, false, 0,
     static_cast<int>(sizeof(kSpaceModControls) /
                      sizeof(kSpaceModControls[0])),
     kSpaceModControls},
    {ModuleId::SpaceFlanger, "Flanger", "Space", "Flanger", false, false, 0,
     static_cast<int>(sizeof(kSpaceModControls) /
                      sizeof(kSpaceModControls[0])),
     kSpaceModControls},
    {ModuleId::SpacePhaser, "Phaser", "Space", "Phaser", false, false, 0,
     static_cast<int>(sizeof(kSpaceModControls) /
                      sizeof(kSpaceModControls[0])),
     kSpaceModControls},
    {ModuleId::SpaceDimension, "Dimension", "Space", "Dimension modulation",
     false, false, 0,
     static_cast<int>(sizeof(kSpaceModControls) /
                      sizeof(kSpaceModControls[0])),
     kSpaceModControls},
};

static_assert(sizeof(kModules) / sizeof(kModules[0]) == kModuleCount,
              "module registry must cover every module id");

inline constexpr const char* kModuleSources[kModuleCount] = {
    "—",
    "Suite Channel",
    "Hearth Channel",
    "Mastering Suite",
    "Vocal Suite",
    "Comp Suite",
    "Comp Suite",
    "Comp Suite",
    "Comp Suite",
    "Comp Suite",
    "Patina",
    "Patina",
    "Patina",
    "Patina",
    "Patina",
    "Suite Channel",
    "Drum Suite",
    "Drum Suite",
    "Vocal Suite",
    "Mastering Suite",
    "Bleed Out / Drum Suite",
    "Brick / Mastering Suite",
    "Suite Rack exclusive",
    "Suite Rack exclusive",
    "Suite Rack exclusive",
    "Suite Rack exclusive",
    "Suite Rack exclusive",
    "Suite Rack exclusive",
};

static_assert(sizeof(kModuleSources) / sizeof(kModuleSources[0]) == kModuleCount,
              "kModuleSources must cover every module id");

} // namespace detail

inline const ModuleDesc* moduleDesc(int id) noexcept {
  return validModule(id) ? &detail::kModules[id] : &detail::kModules[0];
}

inline const char* moduleName(int id) noexcept { return moduleDesc(id)->name; }

inline const char* moduleSource(int id) noexcept {
  return validModule(id) ? detail::kModuleSources[id] : "—";
}

inline void applyModuleDefaults(Shared& shared, int slot, int type,
                                bool markDirty = true) {
  const ModuleDesc* d = moduleDesc(type);
  const ModuleId id = d->id;
  auto set = [&](uint32_t param, double value) {
    if (markDirty) {
      shared.set(param, value);
    } else {
      shared.setFromHost(param, value);
    }
  };
  set(slotParam(slot, kSlotOffsetType), static_cast<double>(type));
  set(slotParam(slot, kSlotOffsetBypass), 0.0);
  set(slotParam(slot, kSlotOffsetMix), 100.0);
  set(slotParam(slot, kSlotOffsetGain), 0.0);
  for (int i = 0; i < kRackControlCount; ++i) {
    set(slotParam(slot, kSlotOffsetC0 + i), defaultControlValue(id, i));
  }
}

inline void initializeRack(Shared& shared) {
  for (int slot = 0; slot < kSlotCount; ++slot) {
    applyModuleDefaults(shared, slot, static_cast<int>(ModuleId::Off), false);
  }
  applyModuleDefaults(shared, 0, static_cast<int>(ModuleId::Eq4K), false);
  applyModuleDefaults(shared, 1, static_cast<int>(ModuleId::Comp76Blue), false);
}

} // namespace rack
