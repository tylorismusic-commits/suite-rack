// Suite Rack — parameters
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

#include "dsp/controls.hpp"

namespace rack {

// Appended-only. Slot blocks are pre-allocated so automation ids survive a
// module change.
enum ParamId : uint32_t {
  P_POWER = 0,
  P_INPUT,
  P_OUTPUT,
  P_SLOT0_TYPE,
  P_SLOT0_BYPASS,
  P_SLOT0_MIX,
  P_SLOT0_EQ_LOW,
  P_SLOT0_EQ_LOW_MID,
  P_SLOT0_EQ_HIGH_MID,
  P_SLOT0_EQ_HIGH,
  P_SLOT0_EQ_HPF,
  P_SLOT0_EQ_LPF,
  P_SLOT0_COMP_MODEL,
  P_SLOT0_COMP_THRESHOLD,
  P_SLOT0_COMP_RATIO,
  P_SLOT0_COMP_ATTACK,
  P_SLOT0_COMP_RELEASE,
  P_SLOT0_COMP_MAKEUP,
  P_SLOT0_COMP_MIX,
  P_SLOT0_COMP_SC_HPF,
  P_SLOT1_TYPE,
  P_SLOT1_BYPASS,
  P_SLOT1_MIX,
  P_SLOT1_EQ_LOW,
  P_SLOT1_EQ_LOW_MID,
  P_SLOT1_EQ_HIGH_MID,
  P_SLOT1_EQ_HIGH,
  P_SLOT1_EQ_HPF,
  P_SLOT1_EQ_LPF,
  P_SLOT1_COMP_MODEL,
  P_SLOT1_COMP_THRESHOLD,
  P_SLOT1_COMP_RATIO,
  P_SLOT1_COMP_ATTACK,
  P_SLOT1_COMP_RELEASE,
  P_SLOT1_COMP_MAKEUP,
  P_SLOT1_COMP_MIX,
  P_SLOT1_COMP_SC_HPF,
  P_SLOT2_TYPE,
  P_SLOT2_BYPASS,
  P_SLOT2_MIX,
  P_SLOT2_EQ_LOW,
  P_SLOT2_EQ_LOW_MID,
  P_SLOT2_EQ_HIGH_MID,
  P_SLOT2_EQ_HIGH,
  P_SLOT2_EQ_HPF,
  P_SLOT2_EQ_LPF,
  P_SLOT2_COMP_MODEL,
  P_SLOT2_COMP_THRESHOLD,
  P_SLOT2_COMP_RATIO,
  P_SLOT2_COMP_ATTACK,
  P_SLOT2_COMP_RELEASE,
  P_SLOT2_COMP_MAKEUP,
  P_SLOT2_COMP_MIX,
  P_SLOT2_COMP_SC_HPF,
  P_SLOT3_TYPE,
  P_SLOT3_BYPASS,
  P_SLOT3_MIX,
  P_SLOT3_EQ_LOW,
  P_SLOT3_EQ_LOW_MID,
  P_SLOT3_EQ_HIGH_MID,
  P_SLOT3_EQ_HIGH,
  P_SLOT3_EQ_HPF,
  P_SLOT3_EQ_LPF,
  P_SLOT3_COMP_MODEL,
  P_SLOT3_COMP_THRESHOLD,
  P_SLOT3_COMP_RATIO,
  P_SLOT3_COMP_ATTACK,
  P_SLOT3_COMP_RELEASE,
  P_SLOT3_COMP_MAKEUP,
  P_SLOT3_COMP_MIX,
  P_SLOT3_COMP_SC_HPF,
  P_SLOT4_TYPE,
  P_SLOT4_BYPASS,
  P_SLOT4_MIX,
  P_SLOT4_EQ_LOW,
  P_SLOT4_EQ_LOW_MID,
  P_SLOT4_EQ_HIGH_MID,
  P_SLOT4_EQ_HIGH,
  P_SLOT4_EQ_HPF,
  P_SLOT4_EQ_LPF,
  P_SLOT4_COMP_MODEL,
  P_SLOT4_COMP_THRESHOLD,
  P_SLOT4_COMP_RATIO,
  P_SLOT4_COMP_ATTACK,
  P_SLOT4_COMP_RELEASE,
  P_SLOT4_COMP_MAKEUP,
  P_SLOT4_COMP_MIX,
  P_SLOT4_COMP_SC_HPF,
  P_SLOT5_TYPE,
  P_SLOT5_BYPASS,
  P_SLOT5_MIX,
  P_SLOT5_EQ_LOW,
  P_SLOT5_EQ_LOW_MID,
  P_SLOT5_EQ_HIGH_MID,
  P_SLOT5_EQ_HIGH,
  P_SLOT5_EQ_HPF,
  P_SLOT5_EQ_LPF,
  P_SLOT5_COMP_MODEL,
  P_SLOT5_COMP_THRESHOLD,
  P_SLOT5_COMP_RATIO,
  P_SLOT5_COMP_ATTACK,
  P_SLOT5_COMP_RELEASE,
  P_SLOT5_COMP_MAKEUP,
  P_SLOT5_COMP_MIX,
  P_SLOT5_COMP_SC_HPF,
  P_COUNT
};

constexpr uint32_t kSlotStride = 17;
constexpr int kSlotOffsetType = 0;
constexpr int kSlotOffsetBypass = 1;
constexpr int kSlotOffsetMix = 2;
constexpr int kSlotOffsetEqLow = 3;
constexpr int kSlotOffsetEqLowMid = 4;
constexpr int kSlotOffsetEqHighMid = 5;
constexpr int kSlotOffsetEqHigh = 6;
constexpr int kSlotOffsetEqHpf = 7;
constexpr int kSlotOffsetEqLpf = 8;
constexpr int kSlotOffsetCompModel = 9;
constexpr int kSlotOffsetCompThreshold = 10;
constexpr int kSlotOffsetCompRatio = 11;
constexpr int kSlotOffsetCompAttack = 12;
constexpr int kSlotOffsetCompRelease = 13;
constexpr int kSlotOffsetCompMakeup = 14;
constexpr int kSlotOffsetCompMix = 15;
constexpr int kSlotOffsetCompScHpf = 16;

inline uint32_t slotParam(int slot, int offset) noexcept {
  return P_SLOT0_TYPE + static_cast<uint32_t>(slot) * kSlotStride +
         static_cast<uint32_t>(offset);
}

struct Labels {
  const char* const* names = nullptr;
  int count = 0;
};

template <size_t N>
constexpr Labels labels(const char* const (&arr)[N]) noexcept {
  return Labels{arr, static_cast<int>(N)};
}

struct ParamSpec {
  const char* symbol;
  const char* name;
  const char* module;
  double min;
  double max;
  double def;
  bool stepped;
  bool hidden;
  bool toggled;
  const char* unit;
  Labels labels;
};

inline constexpr const char* kModuleLabels[] = {"Off", "EQ", "Comp"};
inline constexpr const char* kCompModelLabels[] = {"76", "2A", "VCA"};

#define SLOT_SPECS(n) \
  {"slot" #n "_type", "Module", "Slot " #n, 0.0, 2.0, n == 0 ? 1.0 : (n == 1 ? 2.0 : 0.0), true, false, false, nullptr, labels(kModuleLabels)}, \
  {"slot" #n "_bypass", "Bypass", "Slot " #n, 0.0, 1.0, 0.0, true, false, true, nullptr, {}}, \
  {"slot" #n "_mix", "Mix", "Slot " #n, 0.0, 100.0, 100.0, false, false, false, "%", {}}, \
  {"slot" #n "_eq_low", "Low", "Slot " #n " EQ", 0.0, 1.0, 0.60, false, false, false, "dB", {}}, \
  {"slot" #n "_eq_low_mid", "Low Mid", "Slot " #n " EQ", 0.0, 1.0, 0.45, false, false, false, "dB", {}}, \
  {"slot" #n "_eq_high_mid", "High Mid", "Slot " #n " EQ", 0.0, 1.0, 0.55, false, false, false, "dB", {}}, \
  {"slot" #n "_eq_high", "High", "Slot " #n " EQ", 0.0, 1.0, 0.57, false, false, false, "dB", {}}, \
  {"slot" #n "_eq_hpf", "HPF", "Slot " #n " EQ", 0.0, 1.0, 0.10, false, false, false, "Hz", {}}, \
  {"slot" #n "_eq_lpf", "LPF", "Slot " #n " EQ", 0.0, 1.0, 1.0, false, false, false, "Hz", {}}, \
  {"slot" #n "_comp_model", "Model", "Slot " #n " Comp", 0.0, 2.0, 0.0, true, false, false, nullptr, labels(kCompModelLabels)}, \
  {"slot" #n "_comp_threshold", "Threshold", "Slot " #n " Comp", -40.0, 0.0, -20.0, false, false, false, "dB", {}}, \
  {"slot" #n "_comp_ratio", "Ratio", "Slot " #n " Comp", 0.0, 8.0, 4.0, true, false, false, nullptr, {}}, \
  {"slot" #n "_comp_attack", "Attack", "Slot " #n " Comp", 0.0, 1.0, 0.25, false, false, false, "%", {}}, \
  {"slot" #n "_comp_release", "Release", "Slot " #n " Comp", 0.0, 1.0, 0.35, false, false, false, "%", {}}, \
  {"slot" #n "_comp_makeup", "Makeup", "Slot " #n " Comp", 0.0, 1.0, 0.80, false, false, false, "dB", {}}, \
  {"slot" #n "_comp_mix", "Mix", "Slot " #n " Comp", 0.0, 1.0, 1.0, false, false, false, "%", {}}, \
  {"slot" #n "_comp_sc_hpf", "SC HPF", "Slot " #n " Comp", 0.0, 1.0, 0.05, false, false, false, "Hz", {}}

inline constexpr ParamSpec kParamSpecs[P_COUNT] = {
    {"power", "Power", "Global", 0.0, 1.0, 1.0, true, false, true, nullptr, {}},
    {"input", "Input", "Global", -18.0, 18.0, 0.0, false, false, false, "dB", {}},
    {"output", "Output", "Global", -18.0, 18.0, 0.0, false, false, false, "dB", {}},
    SLOT_SPECS(0),
    SLOT_SPECS(1),
    SLOT_SPECS(2),
    SLOT_SPECS(3),
    SLOT_SPECS(4),
    SLOT_SPECS(5),
};

#undef SLOT_SPECS

static_assert(sizeof(kParamSpecs) / sizeof(kParamSpecs[0]) == P_COUNT,
              "kParamSpecs must cover every parameter id");

class Shared {
public:
  Shared() {
    for (uint32_t p = 0; p < P_COUNT; ++p) {
      values_[p].store(kParamSpecs[p].def, std::memory_order_relaxed);
      dirty_[p].store(false, std::memory_order_relaxed);
    }
    inPeakDb.store(-120.0f);
    outPeakDb.store(-120.0f);
    inPeakLDb.store(-120.0f);
    inPeakRDb.store(-120.0f);
    outPeakLDb.store(-120.0f);
    outPeakRDb.store(-120.0f);
    for (auto& g : slotGrDb) g.store(0.0f);
  }

  double get(uint32_t p) const noexcept {
    return values_[p].load(std::memory_order_relaxed);
  }

  void set(uint32_t p, double v) noexcept {
    if (!std::isfinite(v)) return;
    const ParamSpec& s = kParamSpecs[p];
    v = std::min(std::max(v, s.min), s.max);
    if (s.stepped) v = static_cast<double>(static_cast<int>(v + (v >= 0 ? 0.5 : -0.5)));
    values_[p].store(v, std::memory_order_relaxed);
    dirty_[p].store(true, std::memory_order_release);
  }

  void setFromHost(uint32_t p, double v) noexcept {
    if (!std::isfinite(v)) return;
    const ParamSpec& s = kParamSpecs[p];
    v = std::min(std::max(v, s.min), s.max);
    if (s.stepped) v = static_cast<double>(static_cast<int>(v + (v >= 0 ? 0.5 : -0.5)));
    values_[p].store(v, std::memory_order_relaxed);
  }

  bool takeDirty(uint32_t p) noexcept {
    return dirty_[p].exchange(false, std::memory_order_acquire);
  }
  bool isDirty(uint32_t p) const noexcept {
    return dirty_[p].load(std::memory_order_acquire);
  }

  bool anyDirty() const noexcept {
    for (uint32_t p = 0; p < P_COUNT; ++p)
      if (dirty_[p].load(std::memory_order_acquire)) return true;
    return false;
  }

  std::atomic<float> inPeakDb, outPeakDb;
  std::atomic<float> inPeakLDb, inPeakRDb;
  std::atomic<float> outPeakLDb, outPeakRDb;
  std::atomic<float> slotGrDb[kSlotCount];

private:
  std::atomic<double> values_[P_COUNT];
  std::atomic<bool> dirty_[P_COUNT];
};

inline Controls controlsFrom(const Shared& s) noexcept {
  Controls c;
  c.power = s.get(P_POWER) > 0.5;
  c.inputDb = static_cast<float>(s.get(P_INPUT));
  c.outputDb = static_cast<float>(s.get(P_OUTPUT));
  for (int i = 0; i < kSlotCount; ++i) {
    SlotControls& d = c.slot[i];
    d.type = static_cast<int>(s.get(slotParam(i, kSlotOffsetType)) + 0.5);
    d.bypass = s.get(slotParam(i, kSlotOffsetBypass)) > 0.5;
    d.mix = static_cast<float>(s.get(slotParam(i, kSlotOffsetMix)) / 100.0);
    d.eqLow = static_cast<float>(s.get(slotParam(i, kSlotOffsetEqLow)));
    d.eqLowMid = static_cast<float>(s.get(slotParam(i, kSlotOffsetEqLowMid)));
    d.eqHighMid = static_cast<float>(s.get(slotParam(i, kSlotOffsetEqHighMid)));
    d.eqHigh = static_cast<float>(s.get(slotParam(i, kSlotOffsetEqHigh)));
    d.eqHpf = static_cast<float>(s.get(slotParam(i, kSlotOffsetEqHpf)));
    d.eqLpf = static_cast<float>(s.get(slotParam(i, kSlotOffsetEqLpf)));
    d.compModel = static_cast<int>(s.get(slotParam(i, kSlotOffsetCompModel)) + 0.5);
    d.compThresholdDb = static_cast<float>(s.get(slotParam(i, kSlotOffsetCompThreshold)));
    d.compRatio = static_cast<float>(s.get(slotParam(i, kSlotOffsetCompRatio)));
    d.compAttack = static_cast<float>(s.get(slotParam(i, kSlotOffsetCompAttack)));
    d.compRelease = static_cast<float>(s.get(slotParam(i, kSlotOffsetCompRelease)));
    d.compMakeup = static_cast<float>(s.get(slotParam(i, kSlotOffsetCompMakeup)));
    d.compMix = static_cast<float>(s.get(slotParam(i, kSlotOffsetCompMix)));
    d.compScHpf = static_cast<float>(s.get(slotParam(i, kSlotOffsetCompScHpf)));
  }
  return c;
}

} // namespace rack
