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
constexpr uint32_t kStateVersion = 2;
constexpr uint32_t kStateMaxParams = 4096;

inline std::vector<uint8_t> serializeState(const Shared& shared, double scale) {
  const uint32_t count = static_cast<uint32_t>(P_COUNT);
  Shared& mutableShared = const_cast<Shared&>(shared);
  mutableShared.abCapture(mutableShared.abActiveSlot());
  std::vector<uint8_t> blob(sizeof(kStateMagic) + sizeof(kStateVersion) +
                            sizeof(count) + sizeof(double) * count +
                            sizeof(double) + sizeof(uint32_t) * 2 +
                            2 * (sizeof(uint8_t) + sizeof(double) * count));
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

  const uint32_t abMagic = 0x52414231;  // "RAB1"
  const int32_t active = mutableShared.abActiveSlot();
  put(&abMagic, sizeof(abMagic));
  put(&active, sizeof(active));
  {
    std::lock_guard<std::mutex> lock(mutableShared.abMutex);
    for (int slot = 0; slot < 2; ++slot) {
      const uint8_t valid = mutableShared.abSlots[slot].valid ? 1 : 0;
      put(&valid, sizeof(valid));
      for (uint32_t i = 0; i < count; ++i) {
        put(&mutableShared.abSlots[slot].values[i], sizeof(double));
      }
    }
  }
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
  if (magic != kStateMagic || version != kStateVersion ||
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

  uint32_t abMagic = 0;
  if (off + sizeof(abMagic) <= size && get(&abMagic, sizeof(abMagic)) &&
      abMagic == 0x52414231) {
    int32_t active = 0;
    if (!get(&active, sizeof(active))) return false;
    std::lock_guard<std::mutex> lock(shared.abMutex);
    for (int slot = 0; slot < 2; ++slot) {
      uint8_t valid = 0;
      if (!get(&valid, sizeof(valid))) return false;
      shared.abSlots[slot].valid = valid != 0;
      for (uint32_t i = 0; i < count; ++i) {
        if (!get(&shared.abSlots[slot].values[i], sizeof(double))) {
          return false;
        }
      }
    }
    shared.abActive.store(active >= 0 && active <= 1 ? active : 0,
                          std::memory_order_relaxed);
  } else {
    shared.abInitFromLive();
  }
  return true;
}

} // namespace rack
