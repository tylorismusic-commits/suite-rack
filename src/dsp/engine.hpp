// Suite Rack — serial rack engine
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/db.hpp>
#include <hdsp/smoother.hpp>

#include "colour_module.hpp"
#include "clip_module.hpp"
#include "comp_module.hpp"
#include "deess_module.hpp"
#include "controls.hpp"
#include "eq_module.hpp"
#include "gate_module.hpp"
#include "module_ids.hpp"
#include "transient_module.hpp"

namespace rack {

class Engine {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    for (auto& eq : eq_) eq.prepare(sr_);
    for (auto& comp : comp_) comp.prepare(sr_);
    for (auto& colour : colour_) colour.prepare(sr_);
    for (auto& gate : gate_) gate.prepare(sr_);
    for (auto& transient : transient_) transient.prepare(sr_);
    for (auto& clip : clip_) clip.prepare(sr_);
    for (auto& deess : deess_) deess.prepare(sr_);
    input_.setTime(15.0f, sr_);
    output_.setTime(15.0f, sr_);
    power_.setTime(20.0f, sr_);
    for (auto& gain : slotGain_) gain.setTime(15.0f, sr_);
    input_.reset(1.0f);
    output_.reset(1.0f);
    power_.reset(1.0f);
    for (auto& gain : slotGain_) gain.reset(1.0f);
    reset();
  }

  void setControls(const Controls& c) noexcept {
    controls_ = c;
    for (int i = 0; i < kSlotCount; ++i) {
      eq_[i].setControls(c.slot[i]);
      comp_[i].setControls(c.slot[i]);
      colour_[i].setControls(c.slot[i]);
      gate_[i].setControls(c.slot[i]);
      transient_[i].setControls(c.slot[i]);
      clip_[i].setControls(c.slot[i]);
      deess_[i].setControls(c.slot[i]);
      slotGain_[i].setTarget(hdsp::dbToGain(c.slot[i].gainDb));
    }
    input_.setTarget(hdsp::dbToGain(c.inputDb));
    output_.setTarget(hdsp::dbToGain(c.outputDb));
    power_.setTarget(c.power ? 1.0f : 0.0f);
  }

  void process(float* left, float* right, int frames, Meters& meters) noexcept {
    float inL = 0.0f, inR = 0.0f, outL = 0.0f, outR = 0.0f;
    float slotGr[kSlotCount] = {};
    for (int i = 0; i < frames; ++i) {
      const float rawL = std::isfinite(left[i]) ? left[i] : 0.0f;
      const float rawR = std::isfinite(right[i]) ? right[i] : 0.0f;
      const float gainIn = input_.next();
      const float dryL = rawL * gainIn;
      const float dryR = rawR * gainIn;
      float l = dryL, r = dryR;
      for (int s = 0; s < kSlotCount; ++s) {
        const SlotControls& c = controls_.slot[s];
        const int type = c.type;
        if (type == static_cast<int>(ModuleId::Off) || c.bypass) continue;
        const float sl = l, sr = r;
        float gr = 0.0f;
        if (moduleIsEq(type)) {
          eq_[s].process(l, r);
        } else if (moduleIsComp(type)) {
          gr = comp_[s].process(l, r);
        } else if (moduleIsColour(type)) {
          colour_[s].process(l, r);
        } else if (moduleIsDynamics(type)) {
          if (type == static_cast<int>(ModuleId::DynGate)) {
            gate_[s].process(l, r);
            gr = gate_[s].lastGrDb();
          } else if (type == static_cast<int>(ModuleId::DynTransient)) {
            transient_[s].process(l, r);
          } else if (type == static_cast<int>(ModuleId::DynClip)) {
            clip_[s].process(l, r);
          } else if (type == static_cast<int>(ModuleId::DynDeEss)) {
            deess_[s].process(l, r);
            gr = deess_[s].lastGrDb();
          }
        } else {
          continue;
        }
        const float mix = std::clamp(c.mix, 0.0f, 1.0f);
        l = sl + (l - sl) * mix;
        r = sr + (r - sr) * mix;
        const float gain = slotGain_[s].next();
        l *= gain;
        r *= gain;
        slotGr[s] = std::max(slotGr[s], gr);
      }
      const float active = power_.next();
      l = dryL + active * (l - dryL);
      r = dryR + active * (r - dryR);
      const float gainOut = output_.next();
      l *= gainOut;
      r *= gainOut;
      left[i] = std::isfinite(l) ? l : 0.0f;
      right[i] = std::isfinite(r) ? r : 0.0f;
      inL = std::max(inL, std::fabs(dryL));
      inR = std::max(inR, std::fabs(dryR));
      outL = std::max(outL, std::fabs(left[i]));
      outR = std::max(outR, std::fabs(right[i]));
    }
    meters.inPeakLDb = hdsp::gainToDb(inL);
    meters.inPeakRDb = hdsp::gainToDb(inR);
    meters.inPeakDb = std::max(meters.inPeakLDb, meters.inPeakRDb);
    meters.outPeakLDb = hdsp::gainToDb(outL);
    meters.outPeakRDb = hdsp::gainToDb(outR);
    meters.outPeakDb = std::max(meters.outPeakLDb, meters.outPeakRDb);
    for (int s = 0; s < kSlotCount; ++s) meters.slotGrDb[s] = slotGr[s];
  }

  void reset() noexcept {
    for (auto& eq : eq_) eq.reset();
    for (auto& comp : comp_) comp.reset();
    for (auto& colour : colour_) colour.reset();
    for (auto& gate : gate_) gate.reset();
    for (auto& transient : transient_) transient.reset();
    for (auto& clip : clip_) clip.reset();
    for (auto& deess : deess_) deess.reset();
    input_.snap();
    output_.snap();
    power_.snap();
    for (auto& gain : slotGain_) gain.snap();
  }

  int latencySamples() const noexcept { return 0; }

private:
  float sr_ = 48000.0f;
  Controls controls_;
  EqModule eq_[kSlotCount];
  CompModule comp_[kSlotCount];
  ColourModule colour_[kSlotCount];
  GateModule gate_[kSlotCount];
  TransientModule transient_[kSlotCount];
  ClipModule clip_[kSlotCount];
  DeEssModule deess_[kSlotCount];
  hdsp::Smoothed input_, output_, power_;
  hdsp::Smoothed slotGain_[kSlotCount];
};

} // namespace rack
