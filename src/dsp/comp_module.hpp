// Suite Rack — compressor module family
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Five 500-series compressor entries share the same seven-control layout:
// 76 Blue, 76 Black, 2A, 3A and VCA. Each browser entry fixes the model, so
// the faceplate stays inside the 500-series control budget.
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/db.hpp>

#include "comp/controls.hpp"
#include "comp/fet.hpp"
#include "comp/optical.hpp"
#include "comp/vca.hpp"
#include "controls.hpp"
#include "module_ids.hpp"

namespace rack {

class CompModule {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    fetBlue_.prepare(sr_, comp::FetLimiter::Variant::Blue);
    fetBlack_.prepare(sr_, comp::FetLimiter::Variant::Black);
    opto2A_.prepare(sr_, comp::OptoLeveler::Variant::LA2A);
    opto3A_.prepare(sr_, comp::OptoLeveler::Variant::LA3A);
    vca_.prepare(sr_);
    reset();
  }

  void setControls(const SlotControls& c) noexcept {
    if (c.type != lastType_) {
      reset();
      lastType_ = c.type;
    }
    comp::Controls cc{};
    const float thr = std::clamp(c.c[kCtlThreshold], 0.0f, 1.0f);
    const float t = -40.0f + 40.0f * thr;
    const float inputDb = -18.0f + 0.45f * (t + 20.0f);
    cc.input = std::pow(std::clamp((inputDb + 80.0f) / 103.0f, 0.0f, 1.0f), 2.0f);
    cc.threshold = std::clamp((-t - 8.0f) / 32.0f, 0.0f, 1.0f);
    cc.ratio = std::clamp(c.c[kCtlRatio] * 8.0f, 0.0f, 8.0f);
    cc.attack = c.c[kCtlAttack];
    cc.release = c.c[kCtlRelease];
    cc.makeup = c.c[kCtlMakeup];
    cc.output = 0.5f;
    cc.sidechainHpf = c.c[kCtlScHpf];
    cc.mix = 1.0f;
    cc.link = 1.0f;
    mix_ = std::clamp(c.c[kCtlMix], 0.0f, 1.0f);

    switch (c.type) {
      case static_cast<int>(ModuleId::Comp76Blue):
        model_ = 0;
        fetBlue_.setControls(cc);
        break;
      case static_cast<int>(ModuleId::Comp76Black):
        model_ = 1;
        fetBlack_.setControls(cc);
        break;
      case static_cast<int>(ModuleId::Comp2A):
        model_ = 2;
        cc.threshold = thr;
        opto2A_.setControls(cc);
        break;
      case static_cast<int>(ModuleId::Comp3A):
        model_ = 3;
        cc.threshold = thr;
        opto3A_.setControls(cc);
        break;
      case static_cast<int>(ModuleId::CompVCA):
      default:
        model_ = 4;
        cc.threshold = thr;
        vca_.setControls(cc);
        break;
    }
    trim_ = model_ == 4 ? 1.0f : hdsp::dbToGain(comp::kModelTrimDb[model_]);
  }

  inline float process(float& l, float& r) noexcept {
    const float dryL = l, dryR = r;
    float gr = 0.0f;
    switch (model_) {
      case 0: gr = fetBlue_.process(l, r); break;
      case 1: gr = fetBlack_.process(l, r); break;
      case 2: gr = opto2A_.process(l, r); break;
      case 3: gr = opto3A_.process(l, r); break;
      default: gr = vca_.process(l, r); break;
    }
    l = dryL + (l * trim_ - dryL) * mix_;
    r = dryR + (r * trim_ - dryR) * mix_;
    return gr;
  }

  void reset() noexcept {
    lastType_ = -1;
    fetBlue_.reset();
    fetBlack_.reset();
    opto2A_.reset();
    opto3A_.reset();
    vca_.reset();
  }

private:
  float sr_ = 48000.0f;
  int lastType_ = -1;
  int model_ = 0;
  float trim_ = 1.0f;
  float mix_ = 1.0f;
  comp::FetLimiter fetBlue_;
  comp::FetLimiter fetBlack_;
  comp::OptoLeveler opto2A_;
  comp::OptoLeveler opto3A_;
  comp::BusVca vca_;
};

} // namespace rack
