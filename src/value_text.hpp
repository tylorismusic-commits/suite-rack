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

inline bool parseParamValue(const Shared& shared, uint32_t id,
                            const std::string& raw, double* out) {
  if (id >= P_COUNT || !out) return false;
  const ParamSpec& s = kParamSpecs[id];
  const std::string text = lowerText(trimText(raw));
  if (text.empty()) return false;
  for (int i = 0; i < s.labels.count; ++i) {
    if (text == lowerText(s.labels.names[i])) {
      *out = s.min + i;
      return true;
    }
  }
  if (text == "on" || text == "true" || text == "yes") {
    *out = 1.0;
    return true;
  }
  if (text == "off" || text == "false" || text == "no") {
    *out = 0.0;
    return true;
  }
  char* end = nullptr;
  double v = std::strtod(text.c_str(), &end);
  if (end == text.c_str()) return false;

  // Map the visible value back to the normalised control where the format is
  // unambiguous. Ratio and ms values fall back to a normalised number.
  if (id >= P_SLOT0_TYPE) {
    const int slot = static_cast<int>((id - P_SLOT0_TYPE) / kSlotStride);
    const int offset = static_cast<int>((id - P_SLOT0_TYPE) % kSlotStride);
    if (slot >= 0 && slot < kSlotCount && offset >= kSlotOffsetC0) {
      const int control = offset - kSlotOffsetC0;
      const int type = static_cast<int>(
          shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
      const ModuleDesc* module = moduleDesc(type);
      const ControlDesc* desc = controlFor(*module, control);
      if (desc) {
        if (desc->labels.count > 0) {
          for (int i = 0; i < desc->labels.count; ++i) {
            if (text == lowerText(desc->labels.names[i])) {
              *out = desc->labels.count > 1
                         ? static_cast<double>(i) / (desc->labels.count - 1)
                         : 0.0;
              return true;
            }
          }
        }
        switch (desc->format) {
          case ControlFormat::ThresholdDb: v = (v + 40.0) / 40.0; break;
          case ControlFormat::GainDb: v = v / 20.0 + 0.5; break;
          case ControlFormat::Percent: v = v / 100.0; break;
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
          default: break;
        }
      }
    }
  }

  v = std::min(std::max(v, s.min), s.max);
  *out = v;
  return true;
}

} // namespace rack
