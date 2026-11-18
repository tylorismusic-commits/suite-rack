// Suite Rack — LV2 plugin
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Control ports mirror the CLAP parameter ids: port = kPortFirstParam + ParamId.
// The meter ports follow the parameter block in the same order as the LV2 UI.

#include <lv2/atom/atom.h>
#include <lv2/core/lv2.h>
#include <lv2/state/state.h>
#include <lv2/urid/urid.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "dsp/engine.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "state.hpp"

namespace {

constexpr const char* kPluginUri =
    "https://github.com/tylorismusic-commits/suite-rack";

enum Port : uint32_t {
  PORT_AUDIO_IN_L = 0,
  PORT_AUDIO_IN_R = 1,
  PORT_AUDIO_OUT_L = 2,
  PORT_AUDIO_OUT_R = 3,
  PORT_FIRST_PARAM = 4,
  PORT_IN_PEAK_L = PORT_FIRST_PARAM + rack::P_COUNT,
  PORT_IN_PEAK_R,
  PORT_OUT_PEAK_L,
  PORT_OUT_PEAK_R,
  PORT_IN_PEAK,
  PORT_OUT_PEAK,
  PORT_SLOT0_GR,
  PORT_SLOT1_GR,
  PORT_SLOT2_GR,
  PORT_SLOT3_GR,
  PORT_SLOT4_GR,
  PORT_SLOT5_GR,
  PORT_LATENCY,
  PORT_COUNT
};

struct Plugin {
  float sampleRate = 48000.0f;
  rack::Shared shared;
  rack::Engine engine;
  rack::Meters meters;

  float* audioInL = nullptr;
  float* audioInR = nullptr;
  float* audioOutL = nullptr;
  float* audioOutR = nullptr;

  float* params[rack::P_COUNT] = {};
  bool portsSeen = false;
  float lastPort[rack::P_COUNT] = {};

  float* inPeakL = nullptr;
  float* inPeakR = nullptr;
  float* outPeakL = nullptr;
  float* outPeakR = nullptr;
  float* inPeak = nullptr;
  float* outPeak = nullptr;
  float* slotGr[rack::kSlotCount] = {};
  float* latency = nullptr;

  const LV2_URID_Map* map = nullptr;
  LV2_URID stateKey = 0;
  LV2_URID chunkType = 0;
};

LV2_Handle instantiate(const LV2_Descriptor*, double rate, const char*,
                       const LV2_Feature* const* features) {
  auto* p = new Plugin();
  p->sampleRate = static_cast<float>(rate);
  for (const LV2_Feature* const* f = features; f && *f; ++f) {
    if (!std::strcmp((*f)->URI, LV2_URID__map) && (*f)->data) {
      p->map =
          static_cast<const LV2_URID_Map*>(static_cast<const void*>((*f)->data));
    }
  }
  if (p->map) {
    p->stateKey = p->map->map(
        p->map->handle,
        "https://github.com/tylorismusic-commits/suite-rack#state");
    p->chunkType = p->map->map(p->map->handle, LV2_ATOM__Chunk);
  }
  p->engine.prepare(p->sampleRate);
  rack::initializeRack(p->shared);
  p->engine.setControls(rack::controlsFrom(p->shared));
  return p;
}

void connectPort(LV2_Handle handle, uint32_t port, void* data) {
  auto* p = static_cast<Plugin*>(handle);
  switch (port) {
    case PORT_AUDIO_IN_L: p->audioInL = static_cast<float*>(data); break;
    case PORT_AUDIO_IN_R: p->audioInR = static_cast<float*>(data); break;
    case PORT_AUDIO_OUT_L: p->audioOutL = static_cast<float*>(data); break;
    case PORT_AUDIO_OUT_R: p->audioOutR = static_cast<float*>(data); break;
    case PORT_IN_PEAK_L: p->inPeakL = static_cast<float*>(data); break;
    case PORT_IN_PEAK_R: p->inPeakR = static_cast<float*>(data); break;
    case PORT_OUT_PEAK_L: p->outPeakL = static_cast<float*>(data); break;
    case PORT_OUT_PEAK_R: p->outPeakR = static_cast<float*>(data); break;
    case PORT_IN_PEAK: p->inPeak = static_cast<float*>(data); break;
    case PORT_OUT_PEAK: p->outPeak = static_cast<float*>(data); break;
    case PORT_SLOT0_GR: p->slotGr[0] = static_cast<float*>(data); break;
    case PORT_SLOT1_GR: p->slotGr[1] = static_cast<float*>(data); break;
    case PORT_SLOT2_GR: p->slotGr[2] = static_cast<float*>(data); break;
    case PORT_SLOT3_GR: p->slotGr[3] = static_cast<float*>(data); break;
    case PORT_SLOT4_GR: p->slotGr[4] = static_cast<float*>(data); break;
    case PORT_SLOT5_GR: p->slotGr[5] = static_cast<float*>(data); break;
    case PORT_LATENCY: p->latency = static_cast<float*>(data); break;
    default:
      if (port >= PORT_FIRST_PARAM &&
          port < PORT_FIRST_PARAM + rack::P_COUNT) {
        p->params[port - PORT_FIRST_PARAM] = static_cast<float*>(data);
      }
      break;
  }
}

void activate(LV2_Handle handle) {
  auto* p = static_cast<Plugin*>(handle);
  p->engine.reset();
  p->engine.setControls(rack::controlsFrom(p->shared));
  if (p->latency) {
    *p->latency = static_cast<float>(p->engine.latencySamples());
  }
}

void deactivate(LV2_Handle) {}

void run(LV2_Handle handle, uint32_t frames) {
  auto* p = static_cast<Plugin*>(handle);
  if (!p->audioInL || !p->audioOutL) return;
  if (!p->audioInR) p->audioInR = p->audioInL;
  if (!p->audioOutR) p->audioOutR = p->audioOutL;

  for (uint32_t i = 0; i < rack::P_COUNT; ++i) {
    float* v = p->params[i];
    if (!v) continue;
    if (!p->portsSeen || *v != p->lastPort[i]) {
      p->shared.setFromHost(i, static_cast<double>(*v));
      p->lastPort[i] = *v;
    }
  }
  p->portsSeen = true;

  p->engine.setControls(rack::controlsFrom(p->shared));

  if (p->audioOutL != p->audioInL) {
    std::memcpy(p->audioOutL, p->audioInL, frames * sizeof(float));
  }
  if (p->audioOutR != p->audioInR) {
    std::memcpy(p->audioOutR, p->audioInR, frames * sizeof(float));
  }

  p->engine.process(p->audioOutL, p->audioOutR, static_cast<int>(frames),
                    p->meters);

  if (p->inPeakL) *p->inPeakL = p->meters.inPeakLDb;
  if (p->inPeakR) *p->inPeakR = p->meters.inPeakRDb;
  if (p->outPeakL) *p->outPeakL = p->meters.outPeakLDb;
  if (p->outPeakR) *p->outPeakR = p->meters.outPeakRDb;
  if (p->inPeak) *p->inPeak = p->meters.inPeakDb;
  if (p->outPeak) *p->outPeak = p->meters.outPeakDb;
  for (int i = 0; i < rack::kSlotCount; ++i) {
    if (p->slotGr[i]) *p->slotGr[i] = p->meters.slotGrDb[i];
  }
  if (p->latency) {
    *p->latency = static_cast<float>(p->engine.latencySamples());
  }

  p->shared.inPeakDb.store(p->meters.inPeakDb, std::memory_order_relaxed);
  p->shared.outPeakDb.store(p->meters.outPeakDb, std::memory_order_relaxed);
  p->shared.inPeakLDb.store(p->meters.inPeakLDb, std::memory_order_relaxed);
  p->shared.inPeakRDb.store(p->meters.inPeakRDb, std::memory_order_relaxed);
  p->shared.outPeakLDb.store(p->meters.outPeakLDb, std::memory_order_relaxed);
  p->shared.outPeakRDb.store(p->meters.outPeakRDb, std::memory_order_relaxed);
  for (int i = 0; i < rack::kSlotCount; ++i) {
    p->shared.slotGrDb[i].store(p->meters.slotGrDb[i],
                                std::memory_order_relaxed);
  }
}

LV2_State_Status stateSave(LV2_Handle handle,
                           LV2_State_Store_Function store,
                           LV2_State_Handle state, uint32_t,
                           const LV2_Feature* const*) {
  auto* p = static_cast<Plugin*>(handle);
  if (!p->chunkType) return LV2_STATE_ERR_NO_FEATURE;
  const std::vector<uint8_t> blob = rack::serializeState(p->shared, 1.0);
  store(state, p->stateKey, blob.data(), blob.size(), p->chunkType,
        LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
  return LV2_STATE_SUCCESS;
}

LV2_State_Status stateRestore(LV2_Handle handle,
                              LV2_State_Retrieve_Function retrieve,
                              LV2_State_Handle state, uint32_t,
                              const LV2_Feature* const*) {
  auto* p = static_cast<Plugin*>(handle);
  uint32_t type = 0, flags = 0;
  size_t size = 0;
  const void* data = retrieve(state, p->stateKey, &size, &type, &flags);
  if (!data) return LV2_STATE_SUCCESS;
  if (type != p->chunkType) return LV2_STATE_ERR_BAD_TYPE;
  if (!rack::deserializeState(data, size, p->shared)) {
    return LV2_STATE_ERR_BAD_TYPE;
  }
  // Force the next run() to push the restored values through the engine even
  // if the host has not refreshed the control ports yet.
  p->portsSeen = false;
  return LV2_STATE_SUCCESS;
}

const LV2_State_Interface kStateInterface = {stateSave, stateRestore};

const void* extensionData(const char* uri) {
  if (uri && !std::strcmp(uri, LV2_STATE__interface)) {
    return &kStateInterface;
  }
  return nullptr;
}

void cleanup(LV2_Handle handle) { delete static_cast<Plugin*>(handle); }

const LV2_Descriptor kDescriptor = {kPluginUri,      instantiate, connectPort,
                                    activate,        run,         deactivate,
                                    cleanup,         extensionData};

} // namespace

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  return index == 0 ? &kDescriptor : nullptr;
}
