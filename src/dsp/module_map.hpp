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
