// Suite Rack — typed parameter values
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "modules.hpp"
#include "params.hpp"

namespace rack {

inline std::string lowerText(std::string s) {
  for (char& c : s)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}

inline std::string trimText(const std::string& s) {
  const size_t a = s.find_first_not_of(" \t");
  if (a == std::string::npos) return {};
  const size_t b = s.find_last_not_of(" \t");
  return s.substr(a, b - a + 1);
}

inline float compRatioValue(float step) noexcept {
  static const float kRatios[9] = {1.5f, 2.0f, 3.0f, 4.0f, 6.0f,
                                   8.0f, 12.0f, 20.0f, 20.0f};
  return kRatios[std::clamp(static_cast<int>(std::lround(step)), 0, 8)];
}

inline std::string formatControlValue(const ModuleDesc& module,
                                      const ControlDesc& control,
                                      double value) {
  char buf[64];
  const float v = static_cast<float>(std::clamp(value, 0.0, 1.0));
  if (control.toggled) return value > 0.5 ? "On" : "Off";
  if (control.labels.count > 0) {
    const int i = std::clamp(
        static_cast<int>(std::lround(v * (control.labels.count - 1))), 0,
        control.labels.count - 1);
    return control.labels.names[i];
  }
  switch (control.format) {
    case ControlFormat::Percent:
      std::snprintf(buf, sizeof(buf), "%.0f%%", v * 100.0f);
      return buf;
    case ControlFormat::ThresholdDb:
      std::snprintf(buf, sizeof(buf), "%+.1f dB", -40.0f + 40.0f * v);
      return buf;
    case ControlFormat::GainDb:
      std::snprintf(buf, sizeof(buf), "%+.1f dB", (v - 0.5f) * 20.0f);
      return buf;
    case ControlFormat::Ratio:
      std::snprintf(buf, sizeof(buf), "%.1f:1",
                    compRatioValue(v * 8.0f));
      return buf;
    case ControlFormat::AttackMs: {
      float ms = 0.0f;
      if (module.model == 0 || module.model == 1) {
        const float step = std::round(v * 6.0f) / 6.0f;
        ms = 0.04f * std::pow(40.0f, step);
      } else {
        ms = 0.1f * std::pow(300.0f, v);
      }
      if (ms < 10.0f) std::snprintf(buf, sizeof(buf), "%.1f ms", ms);
      else std::snprintf(buf, sizeof(buf), "%.0f ms", ms);
      return buf;
    }
    case ControlFormat::ReleaseMs: {
      float ms = 0.0f;
      if (module.model == 0 || module.model == 1) {
        const float step = std::round(v * 6.0f) / 6.0f;
        ms = 50.0f * std::pow(22.0f, step);
      } else {
        ms = 100.0f * std::pow(12.0f, v);
      }
      if (ms < 10.0f) std::snprintf(buf, sizeof(buf), "%.1f ms", ms);
      else std::snprintf(buf, sizeof(buf), "%.0f ms", ms);
      return buf;
    }
    case ControlFormat::ScHpfHz:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", 20.0 * std::pow(20.0, v));
      return buf;
    case ControlFormat::EqHpfHz:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", eqHpfHz(v));
      return buf;
    case ControlFormat::EqLpfHz:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", eqLpfHz(v));
      return buf;
    case ControlFormat::AirFreqHz:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", airFreqHz(v));
      return buf;
    case ControlFormat::TiltDb:
    case ControlFormat::AirDb:
      std::snprintf(buf, sizeof(buf), "%+.1f dB", (v - 0.5f) * 24.0f);
      return buf;
    case ControlFormat::GateThresholdDb:
      std::snprintf(buf, sizeof(buf), "%.0f dB", -60.0f + 60.0f * v);
      return buf;
    case ControlFormat::RangeDb:
      std::snprintf(buf, sizeof(buf), "%.0f dB", v * 80.0f);
      return buf;
    case ControlFormat::GateAttackMs: {
      const float ms = 0.1f * std::pow(500.0f, v);
      std::snprintf(buf, sizeof(buf), ms < 10.0f ? "%.1f ms" : "%.0f ms", ms);
      return buf;
    }
    case ControlFormat::GateReleaseMs: {
      const float ms = 10.0f * std::pow(100.0f, v);
      std::snprintf(buf, sizeof(buf), "%.0f ms", ms);
      return buf;
    }
    case ControlFormat::HoldMs:
      std::snprintf(buf, sizeof(buf), "%.0f ms", v * 500.0f);
      return buf;
    case ControlFormat::BipolarPercent:
      std::snprintf(buf, sizeof(buf), "%+.0f%%", (v - 0.5f) * 200.0f);
      return buf;
    case ControlFormat::ClipDriveDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", v * 24.0f);
      return buf;
    case ControlFormat::ClipCeilingDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -12.0f + 12.0f * v);
      return buf;
    case ControlFormat::ClipMode: {
      static const char* names[3] = {"Soft", "Hard", "Diode"};
      const int mode = std::clamp(static_cast<int>(std::lround(v * 2.0f)), 0, 2);
      return names[mode];
    }
    case ControlFormat::DeEssFreqHz:
      std::snprintf(buf, sizeof(buf), "%.1f kHz", 4.0f * std::pow(3.0f, v));
      return buf;
    case ControlFormat::DeEssRangeDb:
      std::snprintf(buf, sizeof(buf), "%.0f dB", v * 24.0f);
      return buf;
    case ControlFormat::LimiterCeilingDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -6.0f + 6.0f * v);
      return buf;
    case ControlFormat::LimiterReleaseMs: {
      const float ms = 20.0f * std::pow(50.0f, v);
      std::snprintf(buf, sizeof(buf), "%.0f ms", ms);
      return buf;
    }
    case ControlFormat::WidthPercent:
      std::snprintf(buf, sizeof(buf), "%.0f%%", v * 200.0f);
      return buf;
    case ControlFormat::MonoHz:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", 20.0f * std::pow(25.0f, v));
      return buf;
    case ControlFormat::CrushBits:
      std::snprintf(buf, sizeof(buf), "%.0f bit", 1.0f + 15.0f * v);
      return buf;
    case ControlFormat::CrushToneHz:
      std::snprintf(buf, sizeof(buf), "%.1f kHz", 1.0f * std::pow(20.0f, v));
      return buf;
    case ControlFormat::FilterCutoffHz:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", 20.0f * std::pow(1000.0f, v));
      return buf;
    case ControlFormat::LfoRateHz:
      std::snprintf(buf, sizeof(buf), "%.2f Hz", 0.05f * std::pow(200.0f, v));
      return buf;
    case ControlFormat::FilterMode: {
      static const char* names[3] = {"LP", "BP", "HP"};
      return names[std::clamp(static_cast<int>(std::lround(v * 2.0f)), 0, 2)];
    }
    case ControlFormat::MotionMode: {
      static const char* names[3] = {"Tremolo", "Auto-Pan", "Rotary"};
      return names[std::clamp(static_cast<int>(std::lround(v * 2.0f)), 0, 2)];
    }
    case ControlFormat::SubFreqHz:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", 30.0f * std::pow(5.0f, v));
      return buf;
    case ControlFormat::DelayMs:
      std::snprintf(buf, sizeof(buf), "%.1f ms", 10.0f + 30.0f * v);
      return buf;
    case ControlFormat::DetuneMs:
      std::snprintf(buf, sizeof(buf), "%.1f ms", 3.0f * v);
      return buf;
    case ControlFormat::QValue:
      std::snprintf(buf, sizeof(buf), "%.2f", 0.3f * std::pow(20.0f, v));
      return buf;
    case ControlFormat::DynRangeDb:
      std::snprintf(buf, sizeof(buf), "%.0f dB", v * 24.0f);
      return buf;
    case ControlFormat::DynRatio:
      std::snprintf(buf, sizeof(buf), "%.1f:1", 1.0f + 9.0f * v);
      return buf;
    case ControlFormat::MakeupDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", v * 24.0f);
      return buf;
    case ControlFormat::RiderTargetDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -40.0f + 40.0f * v);
      return buf;
    case ControlFormat::RiderSpeedMs:
      std::snprintf(buf, sizeof(buf), "%.0f ms", 20.0f * std::pow(50.0f, v));
      return buf;
    case ControlFormat::SpaceDecayS:
      std::snprintf(buf, sizeof(buf), "%.2f s", 0.3f + 11.7f * v);
      return buf;
    case ControlFormat::SpacePreDelayMs:
      std::snprintf(buf, sizeof(buf), "%.0f ms", v * 200.0f);
      return buf;
    case ControlFormat::SpaceDelayMs:
      std::snprintf(buf, sizeof(buf), "%.0f ms", 1.0f + 1999.0f * v);
      return buf;
    case ControlFormat::CompDistortion: {
      static const char* names[4] = {"Off", "Dist 2", "Dist 3", "Clip"};
      return names[std::clamp(static_cast<int>(std::lround(v * 3.0f)), 0, 3)];
    }
    case ControlFormat::BoostDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", v * 12.0f);
      return buf;
    case ControlFormat::ClassicThresholdDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -20.0f + 20.0f * v);
      return buf;
    case ControlFormat::TrimDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -21.0f + 21.0f * v);
      return buf;
    case ControlFormat::DaTubeOutputDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -24.0f + 24.0f * v);
      return buf;
    case ControlFormat::FattenerGainDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -24.0f + 48.0f * v);
      return buf;
    case ControlFormat::TapeColor: {
      static const char* names[3] = {"Normal", "Medium", "Bright"};
      return names[std::clamp(static_cast<int>(std::lround(v * 2.0f)), 0, 2)];
    }
    case ControlFormat::WideGainDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -24.0f + 48.0f * v);
      return buf;
    case ControlFormat::CompAmount:
      std::snprintf(buf, sizeof(buf), "%.1f", 1.0f + 9.0f * v);
      return buf;
    case ControlFormat::AttackShortMs:
      std::snprintf(buf, sizeof(buf), "%.1f ms", 0.1f + 9.9f * v);
      return buf;
    case ControlFormat::AC1Mode: {
      static const char* names[2] = {"AC1", "AC101"};
      return names[std::clamp(static_cast<int>(std::lround(v)), 0, 1)];
    }
    case ControlFormat::Threshold48Db:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -48.0f + 48.0f * v);
      return buf;
    case ControlFormat::Threshold18Db:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -18.0f + 18.0f * v);
      return buf;
    case ControlFormat::CurveBipolar:
      std::snprintf(buf, sizeof(buf), "%+.0f", -50.0f + 100.0f * v);
      return buf;
    case ControlFormat::BloomInputDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -6.0f + 18.0f * v);
      return buf;
    case ControlFormat::BloomOutputDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -12.0f + 12.0f * v);
      return buf;
    case ControlFormat::RsInputDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -24.0f + 48.0f * v);
      return buf;
    case ControlFormat::RsOutputDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -30.0f + 30.0f * v);
      return buf;
    case ControlFormat::Ratio2To10:
      std::snprintf(buf, sizeof(buf), "%.1f : 1", 2.0f + 8.0f * v);
      return buf;
    case ControlFormat::ThresholdM40P10Db:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -40.0f + 50.0f * v);
      return buf;
    case ControlFormat::Unit10:
      std::snprintf(buf, sizeof(buf), "%.1f", 10.0f * v);
      return buf;
    case ControlFormat::Output24Db:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -24.0f + 48.0f * v);
      return buf;
    case ControlFormat::TapeGainDb:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -12.0f + 18.0f * v);
      return buf;
    case ControlFormat::Comp48Db:
      std::snprintf(buf, sizeof(buf), "%.1f dB", 48.0f * v);
      return buf;
    case ControlFormat::Threshold30Db:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -30.0f + 30.0f * v);
      return buf;
    case ControlFormat::Threshold20Db:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -20.0f + 20.0f * v);
      return buf;
    case ControlFormat::Margin6Db:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -6.0f + 6.0f * v);
      return buf;
    case ControlFormat::HybridLowCut:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", 20.0f * std::pow(40.0f, v));
      return buf;
    case ControlFormat::HybridLowFreq:
      std::snprintf(buf, sizeof(buf), "%.0f Hz",
                    30.0f * std::pow(13.3333f, v));
      return buf;
    case ControlFormat::HybridBoost15:
      std::snprintf(buf, sizeof(buf), "%.1f dB", 15.0f * v);
      return buf;
    case ControlFormat::HybridMidFreq:
      std::snprintf(buf, sizeof(buf), "%.0f Hz", 200.0f * std::pow(25.0f, v));
      return buf;
    case ControlFormat::HybridGain15:
      std::snprintf(buf, sizeof(buf), "%+.1f dB", -15.0f + 30.0f * v);
      return buf;
    case ControlFormat::HybridHighFreq:
      std::snprintf(buf, sizeof(buf), "%.0f Hz",
                    2500.0f * std::pow(6.4f, v));
      return buf;
    case ControlFormat::HybridHighBoost:
      std::snprintf(buf, sizeof(buf), "%.1f dB", 18.0f * v);
      return buf;
    case ControlFormat::HybridHighAtten:
      std::snprintf(buf, sizeof(buf), "%.1f dB", -18.0f * v);
      return buf;
    case ControlFormat::L2ReleaseMs: {
      const double ms = std::pow(10.0, 5.0 * static_cast<double>(v) - 5.0) * 1000.0;
      if (ms < 10.0)
        std::snprintf(buf, sizeof(buf), "%.2f ms", ms);
      else if (ms < 100.0)
        std::snprintf(buf, sizeof(buf), "%.1f ms", ms);
      else
        std::snprintf(buf, sizeof(buf), "%.0f ms", ms);
      return buf;
    }
  }
  std::snprintf(buf, sizeof(buf), "%.2f", v);
  return buf;
}

inline const ControlDesc* controlFor(const ModuleDesc& module, int param) {
  for (int i = 0; i < module.controlCount; ++i) {
    if (module.controls[i].param == param) return &module.controls[i];
  }
  return nullptr;
}

inline std::string formatParamValue(const Shared& shared, uint32_t id,
                                    double value) {
  if (id >= P_COUNT) return {};
  const ParamSpec& s = kParamSpecs[id];
  if (s.labels.count > 0) {
    const int i = std::clamp(static_cast<int>(std::lround(value - s.min)), 0,
                             s.labels.count - 1);
    return s.labels.names[i];
  }
  if (s.toggled) return value > 0.5 ? "On" : "Off";

  // Slot generic controls are formatted by the active module.
  if (id >= P_SLOT0_TYPE) {
    const int slot = static_cast<int>((id - P_SLOT0_TYPE) / kSlotStride);
    const int offset = static_cast<int>((id - P_SLOT0_TYPE) % kSlotStride);
    if (slot >= 0 && slot < kSlotCount) {
      if (offset == kSlotOffsetMix) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0f%%", value);
        return buf;
      }
      if (offset == kSlotOffsetGain) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%+.1f dB", value);
        return buf;
      }
      if (offset >= kSlotOffsetC0) {
        const int control = offset - kSlotOffsetC0;
        const int type = static_cast<int>(
            shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
        const ModuleDesc* module = moduleDesc(type);
        const ControlDesc* desc = controlFor(*module, control);
        if (desc) return formatControlValue(*module, *desc, value);
      }
    }
  }

  char buf[64];
  if (s.unit && std::strcmp(s.unit, "dB") == 0) {
    std::snprintf(buf, sizeof(buf), "%+.1f dB", value);
    return buf;
  }
  if (s.unit && std::strcmp(s.unit, "%") == 0) {
    std::snprintf(buf, sizeof(buf), "%.0f%%", value);
    return buf;
  }
  std::snprintf(buf, sizeof(buf), "%.2f", value);
  return buf;
}

inline bool parseNumberText(const std::string& text, double* value,
                            std::string* suffix) {
  const char* start = text.c_str();
  char* end = nullptr;
  const double v = std::strtod(start, &end);
  if (end == start) return false;
  *value = v;
  if (suffix) {
    *suffix = lowerText(trimText(text.substr(static_cast<size_t>(end - start))));
  }
  return true;
}

inline bool labelNumericValue(const char* label, double* value) {
  const char* start = label;
  char* end = nullptr;
  double v = std::strtod(start, &end);
  if (end == start) return false;
  const std::string suffix = lowerText(trimText(std::string(end)));
  if (!suffix.empty() && suffix[0] == 'k') v *= 1000.0;
  *value = v;
  return true;
}

inline size_t editDistance(const std::string& a, const std::string& b) {
  std::vector<size_t> row(b.size() + 1);
  for (size_t j = 0; j <= b.size(); ++j) row[j] = j;
  for (size_t i = 1; i <= a.size(); ++i) {
    size_t prev = row[0];
    row[0] = i;
    for (size_t j = 1; j <= b.size(); ++j) {
      const size_t saved = row[j];
      const size_t cost = a[i - 1] == b[j - 1] ? 0 : 1;
      row[j] = std::min({row[j] + 1, row[j - 1] + 1, prev + cost});
      prev = saved;
    }
  }
  return row[b.size()];
}

inline int closestLabelIndex(const char* const* names, int count,
                             const std::string& text) {
  int best = -1;
  size_t bestScore = static_cast<size_t>(-1);
  for (int i = 0; i < count; ++i) {
    const std::string name = lowerText(names[i]);
    size_t score = 0;
    if (name == text) {
      score = 0;
    } else if (name.rfind(text, 0) == 0) {
      score = 10 + (name.size() - text.size());
    } else if (name.find(text) != std::string::npos) {
      score = 40 + (name.size() - text.size());
    } else {
      score = 100 + editDistance(name, text);
    }
    if (score < bestScore) {
      bestScore = score;
      best = i;
    }
  }
  return best;
}

inline int choiceIndexForText(const char* const* names, int count,
                              const std::string& text, bool toggled) {
  if (!names || count <= 0) return -1;
  for (int i = 0; i < count; ++i) {
    if (text == lowerText(names[i])) return i;
  }

  double typed = 0.0;
  const bool numeric = parseNumberText(text, &typed, nullptr);
  if (numeric && toggled) return typed != 0.0 ? 1 : 0;

  bool hasAlpha = false;
  for (int i = 0; i < count; ++i) {
    for (const char* c = names[i]; *c; ++c) {
      if (std::isalpha(static_cast<unsigned char>(*c))) hasAlpha = true;
    }
  }
  if (numeric && hasAlpha && text.size() > 1) {
    for (int i = 0; i < count; ++i) {
      const std::string name = lowerText(names[i]);
      if (name.rfind(text, 0) == 0) return i;
    }
  }
  if (numeric) {
    int best = -1;
    double bestDelta = 1.0e300;
    for (int i = 0; i < count; ++i) {
      double labelled = 0.0;
      if (!labelNumericValue(names[i], &labelled)) continue;
      const double delta = std::fabs(labelled - typed);
      if (delta < bestDelta) {
        bestDelta = delta;
        best = i;
      }
    }
    if (best >= 0) return best;
    if (typed >= 1.0 && typed <= static_cast<double>(count) &&
        std::fabs(typed - std::round(typed)) < 1.0e-6) {
      return static_cast<int>(std::lround(typed)) - 1;
    }
  }
  return closestLabelIndex(names, count, text);
}

inline bool isFrequencyFormat(ControlFormat format) {
  switch (format) {
    case ControlFormat::ScHpfHz:
    case ControlFormat::EqHpfHz:
    case ControlFormat::EqLpfHz:
    case ControlFormat::AirFreqHz:
    case ControlFormat::DeEssFreqHz:
    case ControlFormat::MonoHz:
    case ControlFormat::CrushToneHz:
    case ControlFormat::FilterCutoffHz:
    case ControlFormat::LfoRateHz:
    case ControlFormat::SubFreqHz:
    case ControlFormat::HybridLowCut:
    case ControlFormat::HybridLowFreq:
    case ControlFormat::HybridMidFreq:
    case ControlFormat::HybridHighFreq:
      return true;
    default:
      return false;
  }
}

inline bool isMillisecondFormat(ControlFormat format) {
  switch (format) {
    case ControlFormat::GateAttackMs:
    case ControlFormat::GateReleaseMs:
    case ControlFormat::HoldMs:
    case ControlFormat::LimiterReleaseMs:
    case ControlFormat::RiderSpeedMs:
    case ControlFormat::SpacePreDelayMs:
    case ControlFormat::SpaceDelayMs:
    case ControlFormat::DelayMs:
    case ControlFormat::DetuneMs:
    case ControlFormat::AttackMs:
    case ControlFormat::ReleaseMs:
    case ControlFormat::L2ReleaseMs:
    case ControlFormat::AttackShortMs:
      return true;
    default:
      return false;
  }
}

inline void applyTypedUnit(ControlFormat format, const std::string& suffix,
                           double* value) {
  if (suffix.empty() || !value) return;
  const bool kilo = suffix[0] == 'k';
  if (isFrequencyFormat(format)) {
    if (kilo) *value *= 1000.0;
    return;
  }
  if (isMillisecondFormat(format)) {
    if (suffix.rfind("ms", 0) == 0) return;
    if (suffix.rfind("us", 0) == 0) {
      *value *= 0.001;
    } else if (suffix[0] == 's') {
      *value *= 1000.0;
    }
    return;
  }
  if (format == ControlFormat::SpaceDecayS) {
    if (suffix.rfind("ms", 0) == 0) {
      *value *= 0.001;
    }
  }
}

inline bool parseParamValue(const Shared& shared, uint32_t id,
                            const std::string& raw, double* out) {
  if (id >= P_COUNT || !out) return false;
  const ParamSpec& specSpec = kParamSpecs[id];
  const std::string text = lowerText(trimText(raw));
  if (text.empty()) return false;

  if (specSpec.labels.count > 0) {
    const int choice = choiceIndexForText(specSpec.labels.names,
                                          specSpec.labels.count, text, false);
    if (choice < 0) return false;
    *out = specSpec.min + choice;
    return true;
  }
  if (text == "on" || text == "true" || text == "yes") {
    *out = 1.0;
    return true;
  }
  if (text == "off" || text == "false" || text == "no") {
    *out = 0.0;
    return true;
  }

  const ModuleDesc* module = nullptr;
  const ControlDesc* desc = nullptr;
  if (id >= P_SLOT0_TYPE) {
    const int slot = static_cast<int>((id - P_SLOT0_TYPE) / kSlotStride);
    const int offset = static_cast<int>((id - P_SLOT0_TYPE) % kSlotStride);
    if (slot >= 0 && slot < kSlotCount && offset >= kSlotOffsetC0) {
      const int control = offset - kSlotOffsetC0;
      const int type = static_cast<int>(
          shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
      module = moduleDesc(type);
      desc = controlFor(*module, control);
    }
  }

  double v = 0.0;
  std::string suffix;
  if (!parseNumberText(text, &v, &suffix)) {
    if (desc && desc->labels.count > 0) {
      const int choice =
          closestLabelIndex(desc->labels.names, desc->labels.count, text);
      if (choice < 0) return false;
      *out = desc->labels.count > 1
                 ? static_cast<double>(choice) / (desc->labels.count - 1)
                 : 0.0;
      return true;
    }
    return false;
  }

  if (desc) {
    if (desc->labels.count > 0) {
      const int choice = choiceIndexForText(
          desc->labels.names, desc->labels.count, text, desc->toggled);
      if (choice < 0) return false;
      if (desc->toggled) {
        *out = choice != 0 ? 1.0 : 0.0;
      } else {
        *out = desc->labels.count > 1
                   ? static_cast<double>(choice) / (desc->labels.count - 1)
                   : 0.0;
      }
      return true;
    }
    applyTypedUnit(desc->format, suffix, &v);
    switch (desc->format) {
      case ControlFormat::ThresholdDb: v = (v + 40.0) / 40.0; break;
      case ControlFormat::GainDb: v = v / 20.0 + 0.5; break;
      case ControlFormat::Ratio: {
        static constexpr double ratios[9] = {1.5, 2.0, 3.0, 4.0, 6.0,
                                             8.0, 12.0, 20.0, 20.0};
        int best = 0;
        double bestDelta = 1.0e300;
        for (int i = 0; i < 9; ++i) {
          const double delta = std::fabs(ratios[i] - v);
          if (delta < bestDelta) {
            bestDelta = delta;
            best = i;
          }
        }
        v = static_cast<double>(best) / 8.0;
        break;
      }
      case ControlFormat::AttackMs: {
        if (module && (module->model == 0 || module->model == 1)) {
          const double step =
              std::log(std::max(v, 0.04) / 0.04) / std::log(40.0);
          v = std::round(step * 6.0) / 6.0;
        } else {
          v = std::log(std::max(v, 0.1) / 0.1) / std::log(300.0);
        }
        break;
      }
      case ControlFormat::ReleaseMs: {
        if (module && (module->model == 0 || module->model == 1)) {
          const double step =
              std::log(std::max(v, 50.0) / 50.0) / std::log(22.0);
          v = std::round(step * 6.0) / 6.0;
        } else {
          v = std::log(std::max(v, 100.0) / 100.0) / std::log(12.0);
        }
        break;
      }
      case ControlFormat::Percent: v = v / 100.0; break;
      case ControlFormat::ScHpfHz:
        v = std::log(std::max(v, 20.0) / 20.0) / std::log(20.0);
        break;
      case ControlFormat::EqHpfHz:
        v = std::log(std::max(v, 20.0) / 20.0) / std::log(25.0);
        break;
      case ControlFormat::EqLpfHz:
        v = std::log(std::max(v, 500.0) / 500.0) / std::log(40.0);
        break;
      case ControlFormat::AirFreqHz:
        v = std::log(std::max(v, 2000.0) / 2000.0) / std::log(8.0);
        break;
      case ControlFormat::TiltDb:
      case ControlFormat::AirDb: v = v / 24.0 + 0.5; break;
      case ControlFormat::GateThresholdDb: v = (v + 60.0) / 60.0; break;
      case ControlFormat::RangeDb: v = v / 80.0; break;
      case ControlFormat::GateAttackMs:
        v = std::log(std::max(v, 0.1) / 0.1) / std::log(500.0);
        break;
      case ControlFormat::GateReleaseMs:
        v = std::log(std::max(v, 10.0) / 10.0) / std::log(100.0);
        break;
      case ControlFormat::HoldMs: v = v / 500.0; break;
      case ControlFormat::BipolarPercent: v = v / 200.0 + 0.5; break;
      case ControlFormat::ClipDriveDb: v = v / 24.0; break;
      case ControlFormat::ClipCeilingDb: v = (v + 12.0) / 12.0; break;
      case ControlFormat::DeEssFreqHz:
        v = std::log(std::max(v, 4.0) / 4.0) / std::log(3.0);
        break;
      case ControlFormat::DeEssRangeDb: v = v / 24.0; break;
      case ControlFormat::LimiterCeilingDb: v = (v + 6.0) / 6.0; break;
      case ControlFormat::LimiterReleaseMs:
        v = std::log(std::max(v, 20.0) / 20.0) / std::log(50.0);
        break;
      case ControlFormat::WidthPercent: v = v / 200.0; break;
      case ControlFormat::MonoHz:
        v = std::log(std::max(v, 20.0) / 20.0) / std::log(25.0);
        break;
      case ControlFormat::CrushBits: v = (v - 1.0) / 15.0; break;
      case ControlFormat::CrushToneHz:
        v = std::log(std::max(v, 1.0) / 1.0) / std::log(20.0);
        break;
      case ControlFormat::FilterCutoffHz:
        v = std::log(std::max(v, 20.0) / 20.0) / std::log(1000.0);
        break;
      case ControlFormat::LfoRateHz:
        v = std::log(std::max(v, 0.05) / 0.05) / std::log(200.0);
        break;
      case ControlFormat::SubFreqHz:
        v = std::log(std::max(v, 30.0) / 30.0) / std::log(5.0);
        break;
      case ControlFormat::DelayMs: v = (v - 10.0) / 30.0; break;
      case ControlFormat::DetuneMs: v = v / 3.0; break;
      case ControlFormat::QValue:
        v = std::log(std::max(v, 0.3) / 0.3) / std::log(20.0);
        break;
      case ControlFormat::DynRangeDb: v = v / 24.0; break;
      case ControlFormat::DynRatio: v = (v - 1.0) / 9.0; break;
      case ControlFormat::MakeupDb: v = v / 24.0; break;
      case ControlFormat::RiderTargetDb: v = (v + 40.0) / 40.0; break;
      case ControlFormat::RiderSpeedMs:
        v = std::log(std::max(v, 20.0) / 20.0) / std::log(50.0);
        break;
      case ControlFormat::SpaceDecayS: v = (v - 0.3) / 11.7; break;
      case ControlFormat::SpacePreDelayMs: v = v / 200.0; break;
      case ControlFormat::SpaceDelayMs: v = (v - 1.0) / 1999.0; break;
      case ControlFormat::BoostDb: v = v / 12.0; break;
      case ControlFormat::ClassicThresholdDb: v = (v + 20.0) / 20.0; break;
      case ControlFormat::TrimDb: v = (v + 21.0) / 21.0; break;
      case ControlFormat::DaTubeOutputDb: v = (v + 24.0) / 24.0; break;
      case ControlFormat::FattenerGainDb: v = (v + 24.0) / 48.0; break;
      case ControlFormat::WideGainDb: v = (v + 24.0) / 48.0; break;
      case ControlFormat::CompAmount: v = (v - 1.0) / 9.0; break;
      case ControlFormat::AttackShortMs: v = (v - 0.1) / 9.9; break;
      case ControlFormat::Threshold48Db: v = (v + 48.0) / 48.0; break;
      case ControlFormat::Threshold18Db: v = (v + 18.0) / 18.0; break;
      case ControlFormat::CurveBipolar: v = (v + 50.0) / 100.0; break;
      case ControlFormat::BloomInputDb: v = (v + 6.0) / 18.0; break;
      case ControlFormat::BloomOutputDb: v = (v + 12.0) / 12.0; break;
      case ControlFormat::RsInputDb: v = (v + 24.0) / 48.0; break;
      case ControlFormat::RsOutputDb: v = (v + 30.0) / 30.0; break;
      case ControlFormat::Ratio2To10: v = (v - 2.0) / 8.0; break;
      case ControlFormat::ThresholdM40P10Db: v = (v + 40.0) / 50.0; break;
      case ControlFormat::Unit10: v = v / 10.0; break;
      case ControlFormat::Output24Db: v = (v + 24.0) / 48.0; break;
      case ControlFormat::TapeGainDb: v = (v + 12.0) / 18.0; break;
      case ControlFormat::Comp48Db: v = v / 48.0; break;
      case ControlFormat::Threshold30Db: v = (v + 30.0) / 30.0; break;
      case ControlFormat::L2ReleaseMs:
        if (v > 0.0) v = (std::log10(v / 1000.0) + 5.0) / 5.0;
        break;
      case ControlFormat::Threshold20Db: v = (v + 20.0) / 20.0; break;
      case ControlFormat::Margin6Db: v = (v + 6.0) / 6.0; break;
      case ControlFormat::HybridLowCut:
        v = std::log(std::max(v, 20.0) / 20.0) / std::log(40.0);
        break;
      case ControlFormat::HybridLowFreq:
        v = std::log(std::max(v, 30.0) / 30.0) / std::log(13.3333);
        break;
      case ControlFormat::HybridBoost15: v = v / 15.0; break;
      case ControlFormat::HybridMidFreq:
        v = std::log(std::max(v, 200.0) / 200.0) / std::log(25.0);
        break;
      case ControlFormat::HybridGain15: v = (v + 15.0) / 30.0; break;
      case ControlFormat::HybridHighFreq:
        v = std::log(std::max(v, 2500.0) / 2500.0) / std::log(6.4);
        break;
      case ControlFormat::HybridHighBoost: v = v / 18.0; break;
      case ControlFormat::HybridHighAtten: v = -v / 18.0; break;
      default: break;
    }
    if (desc->toggled) v = v > 0.5 ? 1.0 : 0.0;
    v = std::min(std::max(v, 0.0), 1.0);
    *out = v;
    return true;
  }

  if (specSpec.stepped) v = std::round(v - specSpec.min) + specSpec.min;
  v = std::min(std::max(v, specSpec.min), specSpec.max);
  *out = v;
  return true;
}

} // namespace rack
