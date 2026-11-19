// Suite Rack — controls
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/modules/eq.hpp>

#include "module_ids.hpp"

namespace rack {

constexpr int kSlotCount = 6;
constexpr int kMeterSpectrumBins = 128;
enum SlotMeterField : int {
  kMeterPeakL = 0,
  kMeterPeakR,
  kMeterHoldL,
  kMeterHoldR,
  kMeterRmsL,
  kMeterRmsR,
  kMeterVu,
  kMeterCorrelation,
  kMeterGr,
  kMeterLufsM,
  kMeterLufsS,
  kSlotMeterFieldCount
};

// Meter-module values published for one slot.  Field order is part of the
// LV2 port order and the UI mapping; append only.
struct SlotMeterValues {
  float peakLDb = -120.0f;
  float peakRDb = -120.0f;
  float holdLDb = -120.0f;
  float holdRDb = -120.0f;
  float rmsLDb = -120.0f;
  float rmsRDb = -120.0f;
  float vu = -20.0f;
  float correlation = 0.0f;
  float grDb = 0.0f;
  float lufsM = -120.0f;
  float lufsS = -120.0f;
};

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
  bool externalSidechain = false;
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
  SlotMeterValues slotMeter[kSlotCount];
  // The last Meter module in the chain supplies the shared spectrum.
  float meterSpectrum[kMeterSpectrumBins] = {};
};

inline float slotMeterFieldValue(const SlotMeterValues& m, int field) noexcept {
  switch (field) {
    case kMeterPeakL: return m.peakLDb;
    case kMeterPeakR: return m.peakRDb;
    case kMeterHoldL: return m.holdLDb;
    case kMeterHoldR: return m.holdRDb;
    case kMeterRmsL: return m.rmsLDb;
    case kMeterRmsR: return m.rmsRDb;
    case kMeterVu: return m.vu;
    case kMeterCorrelation: return m.correlation;
    case kMeterGr: return m.grDb;
    case kMeterLufsM: return m.lufsM;
    case kMeterLufsS: return m.lufsS;
    default: return 0.0f;
  }
}

} // namespace rack
