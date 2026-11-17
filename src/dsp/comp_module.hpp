// Suite Rack — 76 / 2A / VCA compressor module
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/db.hpp>

#include "comp/controls.hpp"
#include "comp/fet.hpp"
#include "comp/optical.hpp"
#include "comp/vca.hpp"
#include "controls.hpp"

namespace rack {

class CompModule {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    fet_.prepare(sr_, comp::FetLimiter::Variant::Blue);
    opto_.prepare(sr_, comp::OptoLeveler::Variant::LA2A);
    vca_.prepare(sr_);
    reset();
  }

  void setControls(const SlotControls& c) noexcept {
    comp::Controls cc{};
    const float t = std::clamp(c.compThresholdDb, -40.0f, 0.0f);
    const float inputDb = -18.0f + 0.45f * (t + 20.0f);
    cc.input = std::pow(std::clamp((inputDb + 80.0f) / 103.0f, 0.0f, 1.0f), 2.0f);
    cc.threshold = std::clamp((-t - 8.0f) / 32.0f, 0.0f, 1.0f);
    cc.ratio = std::clamp(c.compRatio, 0.0f, 8.0f);
    cc.attack = c.compAttack;
    cc.release = c.compRelease;
    cc.makeup = c.compMakeup;
    cc.output = 0.5f;
    cc.sidechainHpf = c.compScHpf;
    cc.link = 1.0f;
    model_ = std::clamp(c.compModel, 0, 2);
    trim_ = model_ == 1 ? 1.0f : hdsp::dbToGain(comp::kModelTrimDb[model_]);
    if (model_ == 0) fet_.setControls(cc);
    else if (model_ == 1) opto_.setControls(cc);
    else {
      comp::Controls vc = cc;
      vc.threshold = std::clamp((t + 40.0f) / 40.0f, 0.0f, 1.0f);
      vca_.setControls(vc);
    }
    mix_ = std::clamp(c.compMix, 0.0f, 1.0f);
  }

  inline float process(float& l, float& r) noexcept {
    const float dryL = l, dryR = r;
    float gr = 0.0f;
    if (model_ == 0) gr = fet_.process(l, r);
    else if (model_ == 1) gr = opto_.process(l, r);
    else gr = vca_.process(l, r);
    l = dryL + (l * trim_ - dryL) * mix_;
    r = dryR + (r * trim_ - dryR) * mix_;
    return gr;
  }

  void reset() noexcept {
    fet_.reset();
    opto_.reset();
    vca_.reset();
  }

private:
  float sr_ = 48000.0f;
  int model_ = 0;
  float trim_ = 1.0f;
  float mix_ = 1.0f;
  comp::FetLimiter fet_;
  comp::OptoLeveler opto_;
  comp::BusVca vca_;
};

} // namespace rack
