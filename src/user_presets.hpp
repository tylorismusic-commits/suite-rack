// Suite Rack — user preset files
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include "params.hpp"

namespace rack {

struct UserPreset {
  std::string name;
  std::vector<double> values;
};

inline std::filesystem::path userPresetDir() {
  const char* xdg = std::getenv("XDG_CONFIG_HOME");
  std::filesystem::path base;
  if (xdg && *xdg) {
    base = xdg;
  } else {
    const char* home = std::getenv("HOME");
    base = home && *home ? std::filesystem::path(home) / ".config"
                         : std::filesystem::path(".");
  }
  return base / "SuiteRack" / "presets";
}

inline std::string sanitizePresetName(const std::string& raw) {
  std::string out;
  for (char c : raw) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (std::isalnum(u) || c == ' ' || c == '-' || c == '_' || c == '.' ||
        c == '(' || c == ')') {
      out.push_back(c);
    } else {
      out.push_back('_');
    }
  }
  const size_t a = out.find_first_not_of(' ');
  if (a == std::string::npos) return {};
  const size_t b = out.find_last_not_of(' ');
  out = out.substr(a, b - a + 1);
  if (out.size() > 60) out.resize(60);
  return out;
}

inline bool saveUserPreset(const std::filesystem::path& dir,
                           const std::string& rawName, const Shared& shared) {
  const std::string name = sanitizePresetName(rawName);
  if (name.empty()) return false;
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (ec) return false;

  const std::filesystem::path finalPath = dir / (name + ".suite-rack");
  const std::filesystem::path tempPath = dir / (name + ".suite-rack.tmp");
  std::ofstream out(tempPath, std::ios::trunc);
  if (!out) return false;
  out << "SuiteRackPreset 1\n";
  out << "name " << name << "\n";
  out.precision(17);
  for (uint32_t i = 0; i < P_COUNT; ++i) {
    out << i << ' ' << shared.get(i) << '\n';
  }
  out.close();
  if (!out) return false;
  std::filesystem::rename(tempPath, finalPath, ec);
  if (ec) {
    std::filesystem::remove(finalPath, ec);
    ec.clear();
    std::filesystem::rename(tempPath, finalPath, ec);
  }
  return !ec;
}

inline std::vector<UserPreset> loadUserPresets(
    const std::filesystem::path& dir) {
  std::vector<UserPreset> out;
  std::error_code ec;
  if (!std::filesystem::is_directory(dir, ec)) return out;
  std::vector<std::filesystem::path> files;
  for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
    if (ec) break;
    if (!entry.is_regular_file()) continue;
    if (entry.path().extension() != ".suite-rack") continue;
    files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());

  for (const auto& path : files) {
    std::ifstream in(path);
    if (!in) continue;
    std::string magic;
    int version = 0;
    in >> magic >> version;
    if (magic != "SuiteRackPreset" || version != 1) continue;
    UserPreset preset;
    preset.values.assign(P_COUNT, 0.0);
    std::string key;
    in >> key;
    if (key == "name") {
      std::getline(in, preset.name);
      const size_t a = preset.name.find_first_not_of(' ');
      preset.name =
          a == std::string::npos ? "" : preset.name.substr(a);
    }
    if (preset.name.empty()) preset.name = path.stem().string();
    int index = 0;
    double value = 0.0;
    while (in >> index >> value) {
      if (index >= 0 && index < static_cast<int>(P_COUNT)) {
        preset.values[static_cast<size_t>(index)] = value;
      }
    }
    out.push_back(std::move(preset));
  }
  return out;
}

} // namespace rack
