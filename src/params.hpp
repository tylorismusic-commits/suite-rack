// Suite Rack — parameters
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

#include "dsp/controls.hpp"
#include "module_ids.hpp"

namespace rack {

// Appended-only. Slot blocks are pre-allocated so automation ids survive a
// module change. Each slot is sixteen ids: type, bypass, mix, gain and twelve
// generic module controls.
enum ParamId : uint32_t {
  P_POWER = 0,
  P_INPUT,
  P_OUTPUT,
#define SLOT_PARAMS(n)                                                        \
  P_SLOT##n##_TYPE, P_SLOT##n##_BYPASS, P_SLOT##n##_MIX, P_SLOT##n##_GAIN,    \
      P_SLOT##n##_C0, P_SLOT##n##_C1, P_SLOT##n##_C2, P_SLOT##n##_C3,        \
      P_SLOT##n##_C4, P_SLOT##n##_C5, P_SLOT##n##_C6, P_SLOT##n##_C7,        \
      P_SLOT##n##_C8, P_SLOT##n##_C9, P_SLOT##n##_C10, P_SLOT##n##_C11,
  SLOT_PARAMS(0)
  SLOT_PARAMS(1)
  SLOT_PARAMS(2)
  SLOT_PARAMS(3)
  SLOT_PARAMS(4)
  SLOT_PARAMS(5)
#undef SLOT_PARAMS
  P_COUNT
};

constexpr uint32_t kSlotStride = 16;
constexpr int kSlotOffsetType = 0;
constexpr int kSlotOffsetBypass = 1;
constexpr int kSlotOffsetMix = 2;
constexpr int kSlotOffsetGain = 3;
constexpr int kSlotOffsetC0 = 4;

constexpr uint32_t slotParam(int slot, int offset) noexcept {
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

#define SLOT_DEFAULT_TYPE(n)                                                  \
  (n == 0 ? static_cast<double>(ModuleId::Eq4K)                               \
          : (n == 1 ? static_cast<double>(ModuleId::Comp76Blue) : 0.0))
#define SLOT_DEFAULT_CTL(n, c)                                                \
  (n == 0 ? defaultControlValue(ModuleId::Eq4K, c)                            \
          : (n == 1 ? defaultControlValue(ModuleId::Comp76Blue, c) : 0.5))

#define SLOT_SPECS(n, display)                                               \
  {"slot" #n "_type", "Module", "Slot " #display, 0.0,                         \
   static_cast<double>(kModuleCount - 1), SLOT_DEFAULT_TYPE(n), true, false,  \
   false, nullptr, labels(kModuleLabels)},                                    \
      {"slot" #n "_bypass", "Bypass", "Slot " #display, 0.0, 1.0, 0.0, true,   \
       false, true, nullptr, {}},                                             \
      {"slot" #n "_mix", "Mix", "Slot " #display, 0.0, 100.0, 100.0, false,  \
       false, false, "%", {}},                                                \
      {"slot" #n "_gain", "Gain", "Slot " #display, -12.0, 12.0, 0.0, false,  \
       false, false, "dB", {}},                                               \
      {"slot" #n "_c0", "Module Control 1", "Slot " #display, 0.0, 1.0,       \
       SLOT_DEFAULT_CTL(n, 0), false, false, false, nullptr, {}},             \
      {"slot" #n "_c1", "Module Control 2", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 1), false, false, false, nullptr, {}},             \
      {"slot" #n "_c2", "Module Control 3", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 2), false, false, false, nullptr, {}},             \
      {"slot" #n "_c3", "Module Control 4", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 3), false, false, false, nullptr, {}},             \
      {"slot" #n "_c4", "Module Control 5", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 4), false, false, false, nullptr, {}},             \
      {"slot" #n "_c5", "Module Control 6", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 5), false, false, false, nullptr, {}},             \
      {"slot" #n "_c6", "Module Control 7", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 6), false, false, false, nullptr, {}},             \
      {"slot" #n "_c7", "Module Control 8", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 7), false, false, false, nullptr, {}},             \
      {"slot" #n "_c8", "Module Control 9", "Slot " #display, 0.0, 1.0,                    \
       SLOT_DEFAULT_CTL(n, 8), false, false, false, nullptr, {}},             \
      {"slot" #n "_c9", "Module Control 10", "Slot " #display, 0.0, 1.0,                   \
       SLOT_DEFAULT_CTL(n, 9), false, false, false, nullptr, {}},             \
      {"slot" #n "_c10", "Module Control 11", "Slot " #display, 0.0, 1.0,                  \
       SLOT_DEFAULT_CTL(n, 10), false, false, false, nullptr, {}},            \
      {"slot" #n "_c11", "Module Control 12", "Slot " #display, 0.0, 1.0,                  \
       SLOT_DEFAULT_CTL(n, 11), false, false, false, nullptr, {}}

inline constexpr ParamSpec kParamSpecs[P_COUNT] = {
    {"power", "Power", "Global", 0.0, 1.0, 1.0, true, false, true, nullptr, {}},
    {"input", "Input", "Global", -18.0, 18.0, 0.0, false, false, false, "dB", {}},
    {"output", "Output", "Global", -18.0, 18.0, 0.0, false, false, false, "dB", {}},
    SLOT_SPECS(0, 1),
    SLOT_SPECS(1, 2),
    SLOT_SPECS(2, 3),
    SLOT_SPECS(3, 4),
    SLOT_SPECS(4, 5),
    SLOT_SPECS(5, 6),
};

#undef SLOT_SPECS
#undef SLOT_DEFAULT_CTL
#undef SLOT_DEFAULT_TYPE

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
    d.gainDb = static_cast<float>(s.get(slotParam(i, kSlotOffsetGain)));
    for (int j = 0; j < kRackControlCount; ++j) {
      d.c[j] = static_cast<float>(s.get(slotParam(i, kSlotOffsetC0 + j)));
    }
  }
  return c;
}

} // namespace rack
