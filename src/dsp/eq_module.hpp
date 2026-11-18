// Suite Rack — EQ module family
// SPDX-License-Identifier: GPL-3.0-or-later
//
// One EQ engine, several 500-series voicings:
//   4K      — console-style four-band EQ
//   Hearth  — inductor console voicing with different corners
//   Tilt    — broad tilt plus filters
//   Air     — high-band lift plus filters
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/biquad.hpp>

#include "controls.hpp"
#include "module_ids.hpp"

namespace rack {

class EqModule {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    reset();
  }

  void setControls(const SlotControls& c) noexcept {
    const int type = c.type;
    if (type != lastType_) {
      resetFilters();
      lastType_ = type;
    }
    if (type == static_cast<int>(ModuleId::Eq4K)) {
      setFourBand(c, 80.0f, 250.0f, 1200.0f, 6000.0f, 0.9f, 0.70f);
    } else if (type == static_cast<int>(ModuleId::EqHearth)) {
      setFourBand(c, 100.0f, 350.0f, 1800.0f, 8000.0f, 0.75f, 0.60f);
    } else if (type == static_cast<int>(ModuleId::EqTilt)) {
      const float tilt = (c.c[0] - 0.5f) * 24.0f;
      lowL_.lowShelf(400.0f, -tilt, 0.5f, sr_);
      lowR_.lowShelf(400.0f, -tilt, 0.5f, sr_);
      highL_.highShelf(2500.0f, tilt, 0.5f, sr_);
      highR_.highShelf(2500.0f, tilt, 0.5f, sr_);
      setFilters(c.c[1], c.c[2]);
    } else if (type == static_cast<int>(ModuleId::EqAir)) {
      const float air = (c.c[0] - 0.5f) * 24.0f;
      const float freq = airFreqHz(c.c[1]);
      highL_.highShelf(freq, air, 0.5f, sr_);
      highR_.highShelf(freq, air, 0.5f, sr_);
      setFilters(c.c[2], 1.0f);
    }
  }

  inline void process(float& l, float& r) noexcept {
    l = lpL_.process(hpL_.process(highL_.process(
        highMidL_.process(lowMidL_.process(lowL_.process(l))))));
    r = lpR_.process(hpR_.process(highR_.process(
        highMidR_.process(lowMidR_.process(lowR_.process(r))))));
  }

  void reset() noexcept {
    resetFilters();
    lastType_ = -1;
  }

private:
  void resetFilters() noexcept {
    lowL_ = hdsp::Biquad{};
    lowR_ = hdsp::Biquad{};
    lowMidL_ = hdsp::Biquad{};
    lowMidR_ = hdsp::Biquad{};
    highMidL_ = hdsp::Biquad{};
    highMidR_ = hdsp::Biquad{};
    highL_ = hdsp::Biquad{};
    highR_ = hdsp::Biquad{};
    hpL_ = hdsp::Biquad{};
    hpR_ = hdsp::Biquad{};
    lpL_ = hdsp::Biquad{};
    lpR_ = hdsp::Biquad{};
  }

  void setFourBand(const SlotControls& c, float lowHz, float lowMidHz,
                   float highMidHz, float highHz, float midQ,
                   float shelfQ) noexcept {
    const float low = (c.c[kCtlLow] - 0.5f) * 24.0f;
    const float lowMid = (c.c[kCtlLowMid] - 0.5f) * 24.0f;
    const float highMid = (c.c[kCtlHighMid] - 0.5f) * 24.0f;
    const float high = (c.c[kCtlHigh] - 0.5f) * 24.0f;
    lowL_.lowShelf(lowHz, low, shelfQ, sr_);
    lowR_.lowShelf(lowHz, low, shelfQ, sr_);
    lowMidL_.peak(lowMidHz, lowMid, midQ, sr_);
    lowMidR_.peak(lowMidHz, lowMid, midQ, sr_);
    highMidL_.peak(highMidHz, highMid, midQ, sr_);
    highMidR_.peak(highMidHz, highMid, midQ, sr_);
    highL_.highShelf(highHz, high, shelfQ, sr_);
    highR_.highShelf(highHz, high, shelfQ, sr_);
    setFilters(c.c[kCtlHpf], c.c[kCtlLpf]);
  }

  void setFilters(float hpf, float lpf) noexcept {
    if (hpf > 0.001f) {
      hpL_.highPass(eqHpfHz(hpf), 0.707f, sr_);
      hpR_.highPass(eqHpfHz(hpf), 0.707f, sr_);
    } else {
      hpL_ = hdsp::Biquad{};
      hpR_ = hdsp::Biquad{};
    }
    if (lpf < 0.999f) {
      lpL_.lowPass(eqLpfHz(lpf), 0.707f, sr_);
      lpR_.lowPass(eqLpfHz(lpf), 0.707f, sr_);
    } else {
      lpL_ = hdsp::Biquad{};
      lpR_ = hdsp::Biquad{};
    }
  }

  float sr_ = 48000.0f;
  int lastType_ = -1;
  hdsp::Biquad lowL_, lowR_, lowMidL_, lowMidR_;
  hdsp::Biquad highMidL_, highMidR_, highL_, highR_;
  hdsp::Biquad hpL_, hpR_, lpL_, lpR_;
};

} // namespace rack
