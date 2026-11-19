// Suite Rack — meter module
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A pass-through rack module that measures the signal at its position in the
// six-slot chain.  Views: peak, VU, spectrum, stereo/correlation, gain
// reduction from earlier dynamics slots, and K-weighted loudness.
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/biquad.hpp>
#include <hdsp/db.hpp>
#include <hdsp/spectrum.hpp>

#include "controls.hpp"

namespace rack {

enum class MeterView : int {
  Peak = 0,
  Vu,
  Spectrum,
  Stereo,
  GainReduction,
  Loudness,
  Count
};

struct MeterControls {
  int view = static_cast<int>(MeterView::Peak);
  int scale = 1;  // 0 narrow, 1 standard, 2 wide
  bool hold = false;
};

class MeterModule {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    spectrum_.prepare(sr_);
    rmsCoeff_ = onePole(0.30f);
    vuCoeff_ = onePole(0.30f);
    momentaryCoeff_ = onePole(0.40f);
    shortTermCoeff_ = onePole(3.00f);
    kShelfL_.highShelf(1681.9745f, 3.99984f, 0.7071752f, sr_);
    kShelfR_ = kShelfL_;
    kHpL_.highPass(38.13547f, 0.5003270f, sr_);
    kHpR_ = kHpL_;
    reset();
  }

  void setControls(const MeterControls& c) noexcept {
    controls_ = c;
    controls_.view =
        std::clamp(controls_.view, 0, static_cast<int>(MeterView::Count) - 1);
    controls_.scale = std::clamp(controls_.scale, 0, 2);
  }

  void setGainReductionDb(float db) noexcept { grDb_ = std::max(db, 0.0f); }

  inline void processSample(float l, float r) noexcept {
    const float al = std::fabs(l);
    const float ar = std::fabs(r);
    peakL_ = std::max(peakL_, al);
    peakR_ = std::max(peakR_, ar);

    rmsL_ += (l * l - rmsL_) * rmsCoeff_;
    rmsR_ += (r * r - rmsR_) * rmsCoeff_;
    const float mono = 0.5f * (l + r);
    vu_ += (mono * mono - vu_) * vuCoeff_;

    corrNum_ += static_cast<double>(l) * r;
    corrL2_ += static_cast<double>(l) * l;
    corrR2_ += static_cast<double>(r) * r;

    const float kl = kHpL_.process(kShelfL_.process(l));
    const float kr = kHpR_.process(kShelfR_.process(r));
    lufsM_ += (kl * kl + kr * kr - lufsM_) * momentaryCoeff_;
    lufsS_ += (kl * kl + kr * kr - lufsS_) * shortTermCoeff_;

    specL_[specCount_] = l;
    specR_[specCount_] = r;
    if (++specCount_ >= kSpecChunk) {
      spectrum_.push(specL_, specR_, specCount_);
      specCount_ = 0;
    }
  }

  void finishBlock(SlotMeterValues& out) noexcept {
    if (specCount_ > 0) {
      spectrum_.push(specL_, specR_, specCount_);
      specCount_ = 0;
    }

    const float pL = hdsp::gainToDb(peakL_);
    const float pR = hdsp::gainToDb(peakR_);
    if (controls_.hold) {
      holdL_ = std::max(holdL_, pL);
      holdR_ = std::max(holdR_, pR);
    } else {
      holdL_ = pL;
      holdR_ = pR;
    }

    out.peakLDb = pL;
    out.peakRDb = pR;
    out.holdLDb = holdL_;
    out.holdRDb = holdR_;
    out.rmsLDb = powerDb(rmsL_);
    out.rmsRDb = powerDb(rmsR_);
    out.vu = std::clamp(powerDb(vu_) + 18.0f, -20.0f, 3.0f);
    out.grDb = grDb_;

    const double denom = std::sqrt(corrL2_ * corrR2_);
    const float blockCorr =
        denom > 1.0e-20 ? static_cast<float>(corrNum_ / denom) : 0.0f;
    corr_ += (blockCorr - corr_) * 0.25f;
    out.correlation = std::clamp(corr_, -1.0f, 1.0f);

    out.lufsM = lufsFromPower(lufsM_);
    out.lufsS = lufsFromPower(lufsS_);

    peakL_ = 0.0f;
    peakR_ = 0.0f;
    corrNum_ = 0.0;
    corrL2_ = 0.0;
    corrR2_ = 0.0;
  }

  const float* spectrumBins() const noexcept { return spectrum_.bins(); }

  int view() const noexcept { return controls_.view; }
  int scale() const noexcept { return controls_.scale; }
  bool hold() const noexcept { return controls_.hold; }

  void reset() noexcept {
    peakL_ = peakR_ = 0.0f;
    holdL_ = holdR_ = -120.0f;
    rmsL_ = rmsR_ = 0.0f;
    vu_ = 0.0f;
    corrNum_ = corrL2_ = corrR2_ = 0.0;
    corr_ = 0.0f;
    lufsM_ = lufsS_ = 0.0f;
    grDb_ = 0.0f;
    specCount_ = 0;
    kShelfL_.reset();
    kShelfR_.reset();
    kHpL_.reset();
    kHpR_.reset();
    spectrum_.reset();
  }

  void resetHolds() noexcept {
    holdL_ = -120.0f;
    holdR_ = -120.0f;
    spectrum_.reset();
  }

private:
  static constexpr int kSpecChunk = 512;

  float onePole(float seconds) const noexcept {
    const float samples = std::max(seconds, 0.001f) * sr_;
    return 1.0f - std::exp(-1.0f / samples);
  }

  static float powerDb(float power) noexcept {
    return power > 1.0e-12f ? std::max(10.0f * std::log10(power), -120.0f)
                            : -120.0f;
  }

  static float lufsFromPower(float power) noexcept {
    return power > 1.0e-12f
               ? std::max(-0.691f + 10.0f * std::log10(power), -120.0f)
               : -120.0f;
  }

  MeterControls controls_;
  float sr_ = 48000.0f;
  hdsp::SpectrumAnalyzer spectrum_;
  hdsp::Biquad kShelfL_, kShelfR_, kHpL_, kHpR_;

  float rmsCoeff_ = 0.0001f;
  float vuCoeff_ = 0.0001f;
  float momentaryCoeff_ = 0.0001f;
  float shortTermCoeff_ = 0.0001f;

  float peakL_ = 0.0f, peakR_ = 0.0f;
  float holdL_ = -120.0f, holdR_ = -120.0f;
  float rmsL_ = 0.0f, rmsR_ = 0.0f;
  float vu_ = 0.0f;
  double corrNum_ = 0.0, corrL2_ = 0.0, corrR2_ = 0.0;
  float corr_ = 0.0f;
  float lufsM_ = 0.0f, lufsS_ = 0.0f;
  float grDb_ = 0.0f;
  float specL_[kSpecChunk] = {};
  float specR_[kSpecChunk] = {};
  int specCount_ = 0;
};

} // namespace rack
