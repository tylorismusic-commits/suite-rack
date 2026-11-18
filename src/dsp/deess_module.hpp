// Suite Rack — de-esser module
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/biquad.hpp>
#include <hdsp/db.hpp>

#include "controls.hpp"

namespace rack {

class DeEssModule {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    attackCoeff_ = coeff(1.0f);
    releaseCoeff_ = coeff(50.0f);
    gainCoeff_ = coeff(10.0f);
    reset();
  }

  void setControls(const SlotControls& c) noexcept {
    const float freq = 4000.0f * std::pow(3.0f, clamp01(c.c[0]));
    hpL_.highPass(freq, 0.707f, sr_);
    hpR_.highPass(freq, 0.707f, sr_);
    threshold_ = hdsp::dbToGain(-40.0f + 40.0f * clamp01(c.c[1]));
    rangeDb_ = clamp01(c.c[2]) * 24.0f;
    mix_ = clamp01(c.c[3]);
  }

  inline void process(float& l, float& r) noexcept {
    const float highL = hpL_.process(l);
    const float highR = hpR_.process(r);
    const float lowL = l - highL;
    const float lowR = r - highR;
    const float detector = std::max(std::fabs(highL), std::fabs(highR));
    const float envCoeff = detector > env_ ? attackCoeff_ : releaseCoeff_;
    env_ += (detector - env_) * envCoeff;

    float reduction = 0.0f;
    if (env_ > threshold_) {
      const float overDb = hdsp::gainToDb(env_ / std::max(threshold_, 1.0e-6f));
      reduction = std::min(overDb, rangeDb_);
    }
    const float target = hdsp::dbToGain(-reduction);
    gain_ += (target - gain_) * gainCoeff_;
    const float wetL = lowL + highL * gain_;
    const float wetR = lowR + highR * gain_;
    const float dryL = l, dryR = r;
    l = dryL + (wetL - dryL) * mix_;
    r = dryR + (wetR - dryR) * mix_;
    lastGrDb_ = -hdsp::gainToDb(std::max(gain_, 1.0e-6f));
  }

  void reset() noexcept {
    hpL_ = hdsp::Biquad{};
    hpR_ = hdsp::Biquad{};
    env_ = 0.0f;
    gain_ = 1.0f;
    lastGrDb_ = 0.0f;
  }

  float lastGrDb() const noexcept { return lastGrDb_; }

private:
  static float clamp01(float x) noexcept {
    return std::min(std::max(x, 0.0f), 1.0f);
  }
  float coeff(float ms) const noexcept {
    const float samples = std::max(ms, 0.01f) * 0.001f * sr_;
    return 1.0f - std::exp(-1.0f / samples);
  }

  float sr_ = 48000.0f;
  float threshold_ = 0.1f;
  float rangeDb_ = 12.0f;
  float mix_ = 1.0f;
  float attackCoeff_ = 0.02f;
  float releaseCoeff_ = 0.001f;
  float gainCoeff_ = 0.002f;
  float env_ = 0.0f;
  float gain_ = 1.0f;
  float lastGrDb_ = 0.0f;
  hdsp::Biquad hpL_, hpR_;
};

} // namespace rack
