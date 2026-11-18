// Suite Rack — state serialization
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The CLAP and LV2 wrappers share this format.  It is deliberately a flat
// POD blob: one magic, one version, the parameter count, the parameter
// values, then the UI scale.  Module blobs will be appended in a later
// version; readers must tolerate trailing bytes so newer states still load
// in older builds where the parameters overlap.
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "params.hpp"

namespace rack {

constexpr uint32_t kStateMagic = 0x53524B32;  // "SRK2"
constexpr uint32_t kStateVersion = 1;
constexpr uint32_t kStateMaxParams = 4096;

inline std::vector<uint8_t> serializeState(const Shared& shared, double scale) {
  const uint32_t count = static_cast<uint32_t>(P_COUNT);
  std::vector<uint8_t> blob(sizeof(kStateMagic) + sizeof(kStateVersion) +
                            sizeof(count) + sizeof(double) * count +
                            sizeof(double));
  size_t off = 0;
  auto put = [&](const void* data, size_t size) {
    std::memcpy(blob.data() + off, data, size);
    off += size;
  };
  put(&kStateMagic, sizeof(kStateMagic));
  put(&kStateVersion, sizeof(kStateVersion));
  put(&count, sizeof(count));
  for (uint32_t i = 0; i < count; ++i) {
    const double value = shared.get(i);
    put(&value, sizeof(value));
  }
  put(&scale, sizeof(scale));
  return blob;
}

inline bool deserializeState(const void* data, size_t size, Shared& shared,
                             double* scaleOut = nullptr) {
  if (!data) {
    return false;
  }
  const uint8_t* bytes = static_cast<const uint8_t*>(data);
  size_t off = 0;
  auto get = [&](void* out, size_t n) {
    if (off + n > size) {
      return false;
    }
    std::memcpy(out, bytes + off, n);
    off += n;
    return true;
  };

  uint32_t magic = 0;
  uint32_t version = 0;
  uint32_t count = 0;
  if (!get(&magic, sizeof(magic)) || !get(&version, sizeof(version)) ||
      !get(&count, sizeof(count))) {
    return false;
  }
  if (magic != kStateMagic || version == 0 || version > kStateVersion ||
      count > kStateMaxParams) {
    return false;
  }

  for (uint32_t i = 0; i < count; ++i) {
    double value = 0.0;
    if (!get(&value, sizeof(value))) {
      return false;
    }
    if (i < static_cast<uint32_t>(P_COUNT)) {
      shared.setFromHost(i, value);
    }
  }

  // The UI scale is optional so a state written by a future build without it
  // still restores the parameters.
  double scale = 0.0;
  if (get(&scale, sizeof(scale)) && scale > 0.0 && scaleOut) {
    *scaleOut = scale;
  }
  return true;
}

} // namespace rack
