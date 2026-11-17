// Suite Rack — 4-band EQ + HPF/LPF module
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/biquad.hpp>

#include "controls.hpp"

namespace rack {

class EqModule {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    reset();
  }

  void setControls(const SlotControls& c) noexcept {
    const float low = (c.eqLow - 0.5f) * 24.0f;
    const float lowMid = (c.eqLowMid - 0.5f) * 24.0f;
    const float highMid = (c.eqHighMid - 0.5f) * 24.0f;
    const float high = (c.eqHigh - 0.5f) * 24.0f;
    lowL_.lowShelf(80.0f, low, 0.7f, sr_);
    lowR_.lowShelf(80.0f, low, 0.7f, sr_);
    lowMidL_.peak(250.0f, lowMid, 0.9f, sr_);
    lowMidR_.peak(250.0f, lowMid, 0.9f, sr_);
    highMidL_.peak(1200.0f, highMid, 0.9f, sr_);
    highMidR_.peak(1200.0f, highMid, 0.9f, sr_);
    highL_.highShelf(6000.0f, high, 0.7f, sr_);
    highR_.highShelf(6000.0f, high, 0.7f, sr_);
    if (c.eqHpf > 0.001f) {
      hpL_.highPass(eqHpfHz(c.eqHpf), 0.707f, sr_);
      hpR_.highPass(eqHpfHz(c.eqHpf), 0.707f, sr_);
    } else {
      hpL_ = hdsp::Biquad{};
      hpR_ = hdsp::Biquad{};
    }
    if (c.eqLpf < 0.999f) {
      lpL_.lowPass(eqLpfHz(c.eqLpf), 0.707f, sr_);
      lpR_.lowPass(eqLpfHz(c.eqLpf), 0.707f, sr_);
    } else {
      lpL_ = hdsp::Biquad{};
      lpR_ = hdsp::Biquad{};
    }
  }

  inline void process(float& l, float& r) noexcept {
    l = lpL_.process(hpL_.process(highL_.process(
        highMidL_.process(lowMidL_.process(lowL_.process(l))))));
    r = lpR_.process(hpR_.process(highR_.process(
        highMidR_.process(lowMidR_.process(lowR_.process(r))))));
  }

  void reset() noexcept {
    lowL_.reset(); lowR_.reset();
    lowMidL_.reset(); lowMidR_.reset();
    highMidL_.reset(); highMidR_.reset();
    highL_.reset(); highR_.reset();
    hpL_.reset(); hpR_.reset();
    lpL_.reset(); lpR_.reset();
  }

private:
  float sr_ = 48000.0f;
  hdsp::Biquad lowL_, lowR_, lowMidL_, lowMidR_;
  hdsp::Biquad highMidL_, highMidR_, highL_, highR_;
  hdsp::Biquad hpL_, hpR_, lpL_, lpR_;
};

} // namespace rack
