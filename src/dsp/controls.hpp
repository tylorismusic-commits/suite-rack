// Suite Rack — controls
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/modules/eq.hpp>

#include "module_ids.hpp"

namespace rack {

constexpr int kSlotCount = 6;

struct SlotControls {
  int type = 0;
  bool bypass = false;
  float mix = 1.0f;
  float gainDb = 0.0f;
  // Sensible neutral defaults for manually constructed controls. The plugin
  // always fills these from the parameter table via controlsFrom().
  float c[kRackControlCount] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 1.0f,
                                0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
};

using hdsp::modules::airFreqHz;
using hdsp::modules::eqHpfHz;
using hdsp::modules::eqLpfHz;

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
