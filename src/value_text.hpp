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

#include "params.hpp"

namespace rack {

inline std::string lowerText(std::string s) {
  for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
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

inline std::string formatParamValue(const Shared& shared, uint32_t id, double value) {
  const ParamSpec& s = kParamSpecs[id];
  if (s.labels.count > 0) {
    const int i = std::clamp(static_cast<int>(std::lround(value - s.min)), 0,
                             s.labels.count - 1);
    return s.labels.names[i];
  }
  char buf[64];
  if (s.toggled) return value > 0.5 ? "On" : "Off";
  if (std::strstr(s.symbol, "eq_low") || std::strstr(s.symbol, "eq_high") ||
      std::strstr(s.symbol, "_eq_low_mid") || std::strstr(s.symbol, "_eq_high_mid")) {
    std::snprintf(buf, sizeof(buf), "%+.1f dB", (value - 0.5) * 24.0);
    return buf;
  }
  if (std::strstr(s.symbol, "eq_hpf"))
    return std::to_string(static_cast<int>(std::lround(eqHpfHz(static_cast<float>(value))))) + " Hz";
  if (std::strstr(s.symbol, "eq_lpf"))
    return std::to_string(static_cast<int>(std::lround(eqLpfHz(static_cast<float>(value))))) + " Hz";
  if (std::strstr(s.symbol, "comp_ratio"))
    return std::to_string(static_cast<int>(std::lround(compRatioValue(static_cast<float>(value))))) + ":1";
  (void)shared;
  if (s.unit && std::strcmp(s.unit, "dB") == 0) {
    if (std::strstr(s.symbol, "makeup"))
      std::snprintf(buf, sizeof(buf), "%+.1f dB", (value - 0.5) * 20.0);
    else
      std::snprintf(buf, sizeof(buf), "%+.1f dB", value);
    return buf;
  }
  if (std::strstr(s.symbol, "comp_attack") ||
      std::strstr(s.symbol, "comp_release")) {
    const int slot = std::clamp(s.symbol[4] - '0', 0, kSlotCount - 1);
    const int model = static_cast<int>(
        shared.get(slotParam(slot, kSlotOffsetCompModel)) + 0.5);
    if (model == 1) return "Program";
    const float v = static_cast<float>(value);
    const float ms = std::strstr(s.symbol, "comp_attack")
                         ? (model == 0 ? 0.04f * std::pow(40.0f, std::round(v * 6.0f) / 6.0f)
                                       : 0.1f * std::pow(300.0f, v))
                         : (model == 0 ? 50.0f * std::pow(22.0f, std::round(v * 6.0f) / 6.0f)
                                       : 100.0f * std::pow(12.0f, v));
    if (ms < 10.0f) std::snprintf(buf, sizeof(buf), "%.1f ms", ms);
    else std::snprintf(buf, sizeof(buf), "%.0f ms", ms);
    return buf;
  }
  if (std::strstr(s.symbol, "comp_mix")) {
    std::snprintf(buf, sizeof(buf), "%.0f%%", value * 100.0);
    return buf;
  }
  if (std::strstr(s.symbol, "comp_sc_hpf")) {
    std::snprintf(buf, sizeof(buf), "%.0f Hz", 20.0 * std::pow(20.0, value));
    return buf;
  }
  if (s.unit && std::strcmp(s.unit, "%") == 0) {
    std::snprintf(buf, sizeof(buf), "%.0f%%", value);
    return buf;
  }
  std::snprintf(buf, sizeof(buf), "%.2f", value);
  return buf;
}

inline bool parseParamValue(const Shared&, uint32_t id, const std::string& raw,
                            double* out) {
  const ParamSpec& s = kParamSpecs[id];
  const std::string text = lowerText(trimText(raw));
  if (text.empty()) return false;
  for (int i = 0; i < s.labels.count; ++i) {
    if (text == lowerText(s.labels.names[i])) {
      *out = s.min + i;
      return true;
    }
  }
  if (text == "on" || text == "true" || text == "yes") { *out = 1.0; return true; }
  if (text == "off" || text == "false" || text == "no") { *out = 0.0; return true; }
  char* end = nullptr;
  double v = std::strtod(text.c_str(), &end);
  if (end == text.c_str()) return false;
  if (std::strstr(s.symbol, "eq_low") || std::strstr(s.symbol, "eq_high")) v = v / 24.0 + 0.5;
  else if (std::strstr(s.symbol, "eq_hpf")) v = std::log(std::max(v, 20.0) / 20.0) / std::log(25.0);
  else if (std::strstr(s.symbol, "eq_lpf")) v = std::log(std::max(v, 500.0) / 500.0) / std::log(40.0);
  else if (std::strstr(s.symbol, "makeup")) v = v / 20.0 + 0.5;
  v = std::min(std::max(v, s.min), s.max);
  *out = v;
  return true;
}

} // namespace rack
