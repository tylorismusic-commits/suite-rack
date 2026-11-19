// Suite Rack — four-knob-per-band multiband compressor
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/crossover.hpp>
#include <hdsp/db.hpp>
#include <hdsp/sidechain.hpp>

namespace rack {

struct MultibandCompControls {
  float threshold[3] = {0.50f, 0.50f, 0.50f};
  float ratio[3] = {0.30f, 0.30f, 0.30f};
  float attack[3] = {0.20f, 0.20f, 0.20f};
  float release[3] = {0.40f, 0.40f, 0.40f};
};

class MultibandComp {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    crossL_.prepare(sr_);
    crossR_.prepare(sr_);
    scL_.prepare(sr_);
    scR_.prepare(sr_);
    crossL_.setCrossovers(200.0f, 2000.0f);
    crossR_.setCrossovers(200.0f, 2000.0f);
    scL_.setCrossovers(200.0f, 2000.0f);
    scR_.setCrossovers(200.0f, 2000.0f);
    reset();
  }

  void setControls(const MultibandCompControls& c) noexcept {
    for (int b = 0; b < 3; ++b) {
      threshold_[b] = -60.0f + 60.0f * clamp01(c.threshold[b]);
      ratio_[b] = 1.0f + 19.0f * clamp01(c.ratio[b]);
      attack_[b] = coeff(0.1f * std::pow(1000.0f, clamp01(c.attack[b])));
      release_[b] = coeff(10.0f * std::pow(100.0f, clamp01(c.release[b])));
    }
  }

  void process(float& l, float& r,
               const hdsp::SidechainFrame& sc = {}) noexcept {
    float lowL, midL, highL, lowR, midR, highR;
    crossL_.process(l, lowL, midL, highL);
    crossR_.process(r, lowR, midR, highR);
    float bandsL[3] = {lowL, midL, highL};
    float bandsR[3] = {lowR, midR, highR};

    float scBands[3] = {0.0f, 0.0f, 0.0f};
    if (sc.active) {
      float sl, sm, sh, sr, smr, shr;
      scL_.process(sc.left, sl, sm, sh);
      scR_.process(sc.right, sr, smr, shr);
      scBands[0] = std::max(std::fabs(sl), std::fabs(sr));
      scBands[1] = std::max(std::fabs(sm), std::fabs(smr));
      scBands[2] = std::max(std::fabs(sh), std::fabs(shr));
    }

    float maxGr = 0.0f;
    for (int b = 0; b < 3; ++b) {
      const float detector =
          sc.active ? scBands[b]
                    : std::max(std::fabs(bandsL[b]), std::fabs(bandsR[b]));
      const float coeff = detector > env_[b] ? attack_[b] : release_[b];
      env_[b] += (detector - env_[b]) * coeff;
      const float envDb = hdsp::gainToDb(std::max(env_[b], 1.0e-6f));
      const float overDb = envDb - threshold_[b];
      float reduction = 0.0f;
      if (overDb > 0.0f) {
        reduction = overDb * (1.0f - 1.0f / ratio_[b]);
      }
      const float gain = hdsp::dbToGain(-reduction);
      bandsL[b] *= gain;
      bandsR[b] *= gain;
      maxGr = std::max(maxGr, reduction);
    }

    l = bandsL[0] + bandsL[1] + bandsL[2];
    r = bandsR[0] + bandsR[1] + bandsR[2];
    lastGrDb_ = maxGr;
  }

  void reset() noexcept {
    crossL_.reset();
    crossR_.reset();
    scL_.reset();
    scR_.reset();
    for (int b = 0; b < 3; ++b) env_[b] = 0.0f;
    lastGrDb_ = 0.0f;
  }

  float lastGrDb() const noexcept { return lastGrDb_; }
  int latencySamples() const noexcept { return 0; }

private:
  static float clamp01(float x) noexcept {
    return std::min(std::max(x, 0.0f), 1.0f);
  }
  float coeff(float ms) const noexcept {
    const float samples = std::max(ms, 0.01f) * 0.001f * sr_;
    return 1.0f - std::exp(-1.0f / samples);
  }

  float sr_ = 48000.0f;
  hdsp::ThreeBandCrossover crossL_, crossR_, scL_, scR_;
  float threshold_[3] = {-30.0f, -30.0f, -30.0f};
  float ratio_[3] = {3.7f, 3.7f, 3.7f};
  float attack_[3] = {0.02f, 0.02f, 0.02f};
  float release_[3] = {0.001f, 0.001f, 0.001f};
  float env_[3] = {0.0f, 0.0f, 0.0f};
  float lastGrDb_ = 0.0f;
};

} // namespace rack
