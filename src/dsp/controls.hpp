// Suite Rack — controls
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

namespace rack {

constexpr int kSlotCount = 6;

enum class ModuleType : int { Off = 0, Eq = 1, Comp = 2 };

struct SlotControls {
  int type = 0;
  bool bypass = false;
  float mix = 1.0f;

  float eqLow = 0.5f;
  float eqLowMid = 0.5f;
  float eqHighMid = 0.5f;
  float eqHigh = 0.5f;
  float eqHpf = 0.0f;
  float eqLpf = 1.0f;

  int compModel = 0;
  float compThresholdDb = -20.0f;
  float compRatio = 4.0f;
  float compAttack = 0.25f;
  float compRelease = 0.35f;
  float compMakeup = 0.8f;
  float compMix = 1.0f;
  float compScHpf = 0.05f;
};

inline float eqHpfHz(float v) noexcept {
  v = std::clamp(v, 0.0f, 1.0f);
  return 20.0f * std::pow(25.0f, v);
}
inline float eqLpfHz(float v) noexcept {
  v = std::clamp(v, 0.0f, 1.0f);
  return 500.0f * std::pow(40.0f, v);
}

struct Controls {
  bool power = true;
  float inputDb = 0.0f;
  float outputDb = 0.0f;
  SlotControls slot[kSlotCount];
};

struct Meters {
  float inPeakDb = -120.0f;
  float outPeakDb = -120.0f;
  float inPeakLDb = -120.0f;
  float inPeakRDb = -120.0f;
  float outPeakLDb = -120.0f;
  float outPeakRDb = -120.0f;
  float slotGrDb[kSlotCount] = {};
};

} // namespace rack
