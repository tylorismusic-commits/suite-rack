// Suite Rack — adapters from rack controls to shared hdsp modules
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The DSP lives in hearth-dsp. This file is the only place that knows how a
// Suite Rack slot maps onto a shared module's typed controls.
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/modules/modules.hpp>

#include "controls.hpp"
#include "module_ids.hpp"

namespace rack {

inline int eqVoicing(int type) noexcept {
  switch (type) {
    case static_cast<int>(ModuleId::Eq4K): return 0;
    case static_cast<int>(ModuleId::EqHearth): return 1;
    case static_cast<int>(ModuleId::EqTilt): return 2;
    case static_cast<int>(ModuleId::EqAir): return 3;
    default: return 0;
  }
}

inline int compModel(int type) noexcept {
  switch (type) {
    case static_cast<int>(ModuleId::Comp76Blue): return 0;
    case static_cast<int>(ModuleId::Comp76Black): return 1;
    case static_cast<int>(ModuleId::Comp2A): return 2;
    case static_cast<int>(ModuleId::Comp3A): return 3;
    case static_cast<int>(ModuleId::CompVCA): return 4;
    case static_cast<int>(ModuleId::HearthComp): return 5;
    case static_cast<int>(ModuleId::Glue): return 6;
    case static_cast<int>(ModuleId::CompXpress): return 7;
    case static_cast<int>(ModuleId::CompDigital): return 8;
    case static_cast<int>(ModuleId::CompMolot): return 9;
    case static_cast<int>(ModuleId::CompRed): return 10;
    default: return 0;
  }
}

inline hdsp::modules::EqControls eqControlsFrom(const SlotControls& c) noexcept {
  hdsp::modules::EqControls out;
  out.voicing = eqVoicing(c.type);
  out.low = c.c[kCtlLow];
  out.lowMid = c.c[kCtlLowMid];
  out.highMid = c.c[kCtlHighMid];
  out.high = c.c[kCtlHigh];
  out.hpf = c.c[kCtlHpf];
  out.lpf = c.c[kCtlLpf];
  return out;
}

inline hdsp::modules::CompControls compControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::CompControls out;
  out.model = compModel(c.type);
  out.threshold = c.c[kCtlThreshold];
  out.ratio = c.c[kCtlRatio];
  out.attack = c.c[kCtlAttack];
  out.release = c.c[kCtlRelease];
  out.makeup = c.c[kCtlMakeup];
  out.mix = c.c[kCtlMix];
  out.scHpf = c.c[kCtlScHpf];
  out.distortion = c.c[7];
  return out;
}

inline hdsp::modules::ColourControls colourControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::ColourControls out;
  out.model = std::clamp(c.type - static_cast<int>(ModuleId::ColourValve), 0, 4);
  out.drive = c.c[0];
  out.character = c.c[1];
  out.tone = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline hdsp::modules::GateControls gateControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::GateControls out;
  out.threshold = c.c[0];
  out.range = c.c[1];
  out.attack = c.c[2];
  out.release = c.c[3];
  out.hold = c.c[4];
  return out;
}

inline hdsp::modules::TransientControls transientControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::TransientControls out;
  out.attack = c.c[0];
  out.sustain = c.c[1];
  out.mix = c.c[2];
  return out;
}

inline hdsp::modules::ClipControls clipControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::ClipControls out;
  out.drive = c.c[0];
  out.ceiling = c.c[1];
  out.mode = std::clamp(
      static_cast<int>(std::lround(std::clamp(c.c[2], 0.0f, 1.0f) * 2.0f)), 0,
      2);
  out.mix = c.c[3];
  return out;
}

inline hdsp::modules::DynamicEqControls dynamicEqControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::DynamicEqControls out;
  out.frequency = c.c[0];
  out.q = c.c[1];
  out.threshold = c.c[2];
  out.range = c.c[3];
  out.ratio = c.c[4];
  out.mix = c.c[5];
  return out;
}

inline hdsp::modules::MultibandControls multibandControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::MultibandControls out;
  out.lowThreshold = c.c[0];
  out.midThreshold = c.c[1];
  out.highThreshold = c.c[2];
  out.ratio = c.c[3];
  out.attack = c.c[4];
  out.release = c.c[5];
  out.makeup = c.c[6];
  return out;
}

inline int spaceReverbMode(int type) noexcept {
  switch (type) {
    case static_cast<int>(ModuleId::SpacePlate): return 0;
    case static_cast<int>(ModuleId::SpaceRoom): return 1;
    case static_cast<int>(ModuleId::SpaceHall): return 2;
    case static_cast<int>(ModuleId::SpaceSpring): return 3;
    default: return 0;
  }
}

inline int spaceDelayMode(int type) noexcept {
  switch (type) {
    case static_cast<int>(ModuleId::SpaceDigital): return 0;
    case static_cast<int>(ModuleId::SpaceTape): return 1;
    case static_cast<int>(ModuleId::SpaceAnalog): return 2;
    case static_cast<int>(ModuleId::SpacePingPong): return 3;
    default: return 0;
  }
}

inline int spaceModMode(int type) noexcept {
  switch (type) {
    case static_cast<int>(ModuleId::SpaceChorus): return 0;
    case static_cast<int>(ModuleId::SpaceFlanger): return 1;
    case static_cast<int>(ModuleId::SpacePhaser): return 2;
    case static_cast<int>(ModuleId::SpaceDimension): return 3;
    default: return 0;
  }
}

inline hdsp::modules::SpaceReverbControls spaceReverbControlsFrom(
    const SlotControls& c, int mode) noexcept {
  hdsp::modules::SpaceReverbControls out;
  out.mode = mode;
  out.size = c.c[0];
  out.decay = c.c[1];
  out.predelay = c.c[2];
  out.tone = c.c[3];
  out.width = c.c[4];
  out.mix = c.c[5];
  return out;
}

inline hdsp::modules::SpaceDelayControls spaceDelayControlsFrom(
    const SlotControls& c, int mode) noexcept {
  hdsp::modules::SpaceDelayControls out;
  out.mode = mode;
  out.time = c.c[0];
  out.feedback = c.c[1];
  out.tone = c.c[2];
  out.mod = c.c[3];
  out.width = c.c[4];
  out.mix = c.c[5];
  return out;
}

inline hdsp::modules::SpaceModControls spaceModControlsFrom(
    const SlotControls& c, int mode) noexcept {
  hdsp::modules::SpaceModControls out;
  out.mode = mode;
  out.rate = c.c[0];
  out.depth = c.c[1];
  out.feedback = c.c[2];
  out.width = c.c[3];
  out.mix = c.c[4];
  return out;
}

inline hdsp::modules::RiderControls riderControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::RiderControls out;
  out.target = c.c[0];
  out.speed = c.c[1];
  out.range = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline hdsp::modules::CleanupControls cleanupControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::CleanupControls out;
  out.declick = c.c[0];
  out.dehum = c.c[1];
  out.mains = c.c[2] > 0.5f ? 1 : 0;
  out.mix = c.c[3];
  return out;
}

inline hdsp::modules::SubControls subControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::SubControls out;
  out.frequency = c.c[0];
  out.amount = c.c[1];
  out.drive = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline hdsp::modules::DoubleControls doubleControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::DoubleControls out;
  out.delay = c.c[0];
  out.detune = c.c[1];
  out.width = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline hdsp::modules::FilterControls filterControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::FilterControls out;
  out.cutoff = c.c[0];
  out.resonance = c.c[1];
  out.envelope = c.c[2];
  out.rate = c.c[3];
  out.mode = std::clamp(
      static_cast<int>(std::lround(std::clamp(c.c[4], 0.0f, 1.0f) * 2.0f)), 0,
      2);
  return out;
}

inline hdsp::modules::MotionControls motionControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::MotionControls out;
  out.rate = c.c[0];
  out.depth = c.c[1];
  out.width = c.c[2];
  out.mode = std::clamp(
      static_cast<int>(std::lround(std::clamp(c.c[3], 0.0f, 1.0f) * 2.0f)), 0,
      2);
  out.mix = c.c[4];
  return out;
}

inline hdsp::modules::ImageControls imageControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::ImageControls out;
  out.width = c.c[0];
  out.mono = c.c[1];
  out.balance = c.c[2];
  out.ms = c.c[3] > 0.5f;
  return out;
}

inline hdsp::modules::CrushControls crushControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::CrushControls out;
  out.bits = c.c[0];
  out.rate = c.c[1];
  out.tone = c.c[2];
  out.mix = c.c[3];
  return out;
}

inline hdsp::modules::LimiterControls limiterControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::LimiterControls out;
  out.input = c.c[0];
  out.ceiling = c.c[1];
  out.release = c.c[2];
  out.link = c.c[3];
  return out;
}

inline hdsp::modules::BleedOutControls bleedOutControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::BleedOutControls out;
  out.profile = std::clamp(
      static_cast<int>(std::lround(std::clamp(c.c[0], 0.0f, 1.0f) * 7.0f)), 0,
      7);
  out.threshold = c.c[1];
  out.deBleed = c.c[2];
  out.ghost = c.c[3];
  out.ghostEnabled = c.c[4] > 0.5f;
  return out;
}

inline hdsp::modules::DensityControls densityControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::DensityControls out;
  out.input = c.c[0];
  out.effect = c.c[1];
  out.output = c.c[2];
  return out;
}

inline hdsp::modules::DeEssControls deEssControlsFrom(
    const SlotControls& c) noexcept {
  hdsp::modules::DeEssControls out;
  out.frequency = c.c[0];
  out.threshold = c.c[1];
  out.range = c.c[2];
  out.mix = c.c[3];
  return out;
}

} // namespace rack
