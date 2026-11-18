// Suite Rack — Patina colour module family
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A single-band, four-control version of the Patina BandSaturator. The five
// browser entries fix the model, so each one stays inside the 500-series
// control budget: Drive, Character, Tone and Mix.
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/adaa.hpp>
#include <hdsp/biquad.hpp>
#include <hdsp/db.hpp>
#include <hdsp/smoother.hpp>

#include "controls.hpp"
#include "module_ids.hpp"

namespace rack {

class ColourModule {
public:
  void prepare(float sampleRate) noexcept {
    rate_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    driveSmooth_.setTime(15.0f, rate_);
    mixSmooth_.setTime(10.0f, rate_);
    reset();
    driveSmooth_.snap();
    mixSmooth_.snap();
  }

  void setControls(const SlotControls& c) noexcept {
    if (c.type != lastType_) {
      reset();
      lastType_ = c.type;
      model_ = std::clamp(c.type - static_cast<int>(ModuleId::ColourValve), 0,
                          kModelCount - 1);
    }
    driveNorm_ = clamp01(c.c[0]);
    character_ = clamp01(c.c[1]);
    toneValue_ = clamp01(c.c[2]);
    driveSmooth_.setTarget(hdsp::dbToGain(-12.0f + 24.0f * driveNorm_));
    mixSmooth_.setTarget(clamp01(c.c[3]));
    setFilters();
  }

  inline void process(float& left, float& right) noexcept {
    const float drive = driveSmooth_.next();
    const float mix = mixSmooth_.next();
    const float dryL = left;
    const float dryR = right;

    const float k = 1.0f + 5.0f * driveNorm_;
    const float trim = hdsp::dbToGain(kModelTrimDb[model_]);

    float xL = pre_[0].process(dryL * drive);
    float xR = pre_[1].process(dryR * drive);
    xL = shaper_[0].process(xL, k, biasFor(0));
    xR = shaper_[1].process(xR, k, biasFor(1));
    xL = tone_[0].process(xL);
    xR = tone_[1].process(xR);
    if (model_ == kTape) {
      xL = lp_[0].process(xL);
      xR = lp_[1].process(xR);
    }
    xL = hp_[0].process(xL);
    xR = hp_[1].process(xR);
    xL *= trim;
    xR *= trim;

    const float yL = dryL + mix * (xL - dryL);
    const float yR = dryR + mix * (xR - dryR);
    left = std::isfinite(yL) ? yL : 0.0f;
    right = std::isfinite(yR) ? yR : 0.0f;
  }

  void reset() noexcept {
    lastType_ = -1;
    for (int ch = 0; ch < 2; ++ch) {
      pre_[ch] = hdsp::Biquad{};
      tone_[ch] = hdsp::Biquad{};
      lp_[ch] = hdsp::Biquad{};
      hp_[ch] = hdsp::Biquad{};
      shaper_[ch].reset();
    }
  }

private:
  static constexpr int kModelCount = 5;
  static constexpr int kTape = 3;

  static float clamp01(float x) noexcept {
    return std::min(std::max(x, 0.0f), 1.0f);
  }

  void setFilters() noexcept {
    const float high = (toneValue_ - 0.5f) * 12.0f;
    const float low = (0.5f - toneValue_) * 8.0f;
    for (int ch = 0; ch < 2; ++ch) {
      float preHz = 160.0f;
      float preDb = low * 0.35f;
      switch (model_) {
        case 1:  // Console
          preHz = 150.0f;
          preDb += 1.5f * character_;
          break;
        case 2:  // Bus
          preHz = 120.0f;
          preDb += 1.0f * character_;
          break;
        case 3:  // Tape
          preHz = 90.0f;
          preDb += 4.0f * character_;
          break;
        case 4:  // Transformer
          preHz = 60.0f;
          preDb += 6.0f * character_;
          break;
        default:  // Valve
          preDb += 1.2f * character_;
          break;
      }
      pre_[ch].lowShelf(preHz, preDb, 0.7f, rate_);
      tone_[ch].highShelf(3500.0f + 1500.0f * toneValue_, high, 0.7f, rate_);
      if (model_ == kTape) {
        lp_[ch].lowPass(12000.0f + 6000.0f * toneValue_, 0.7071f, rate_);
      } else {
        lp_[ch].lowPass(21000.0f, 0.7071f, rate_);
      }
      hp_[ch].highPass(model_ == 4 ? 22.0f : 10.0f, 0.7071f, rate_);
    }
  }

  float biasFor(int ch) const noexcept {
    float bias = 0.10f + 0.20f * character_;
    switch (model_) {
      case 1:
        bias = 0.04f + 0.10f * character_ +
               (ch == 0 ? 0.02f : -0.02f) * character_;
        break;
      case 2:
        bias = 0.02f + 0.08f * character_;
        break;
      case 3:
        bias = 0.06f + 0.12f * character_;
        break;
      case 4:
        bias = 0.14f + 0.22f * character_;
        break;
      default:
        break;
    }
    return bias;
  }

  static constexpr float kModelTrimDb[kModelCount] = {0.0f, -1.0f, -1.5f,
                                                      -1.0f, -1.0f};

  float rate_ = 48000.0f;
  int model_ = 0;
  int lastType_ = -1;
  float driveNorm_ = 0.45f;
  float character_ = 0.5f;
  float toneValue_ = 0.5f;
  hdsp::Smoothed driveSmooth_;
  hdsp::Smoothed mixSmooth_;
  hdsp::Biquad pre_[2];
  hdsp::Biquad tone_[2];
  hdsp::Biquad lp_[2];
  hdsp::Biquad hp_[2];
  hdsp::AsymShaper shaper_[2];
};

} // namespace rack
