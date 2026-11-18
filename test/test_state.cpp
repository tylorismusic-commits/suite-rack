// Suite Rack — state and automation tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <cstdio>
#include <vector>

#include "params.hpp"
#include "state.hpp"

using namespace rack;

namespace {
int fails = 0;

void check(bool ok, const char* what) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) ++fails;
}

bool sameValues(const Shared& a, const Shared& b) {
  for (uint32_t i = 0; i < P_COUNT; ++i) {
    if (std::fabs(a.get(i) - b.get(i)) > 1e-9) return false;
  }
  return true;
}
} // namespace

int main() {
  {
    Shared source;
    source.setFromHost(P_INPUT, 3.5);
    source.setFromHost(P_SLOT0_TYPE, 2.0);
    source.setFromHost(P_SLOT0_COMP_THRESHOLD, -27.0);
    source.setFromHost(P_SLOT1_TYPE, 1.0);
    source.setFromHost(P_SLOT1_EQ_HPF, 0.64);
    source.setFromHost(P_SLOT5_MIX, 42.0);

    const std::vector<uint8_t> blob = serializeState(source, 1.25);
    check(blob.size() > 0, "state serializes");

    Shared restored;
    double scale = 0.0;
    check(deserializeState(blob.data(), blob.size(), restored, &scale),
          "state deserializes");
    check(sameValues(source, restored), "state round-trips every parameter");
    check(std::fabs(scale - 1.25) < 1e-9, "state round-trips the UI scale");
  }
  {
    Shared source;
    source.setFromHost(P_SLOT0_TYPE, 2.0);
    const std::vector<uint8_t> blob = serializeState(source, 1.0);
    Shared restored;
    check(!deserializeState(blob.data(), 4, restored),
          "truncated state is rejected");
    std::vector<uint8_t> corrupt = blob;
    corrupt[0] ^= 0xff;
    check(!deserializeState(corrupt.data(), corrupt.size(), restored),
          "bad magic is rejected");
  }
  {
    Shared s;
    check(!s.isDirty(P_INPUT), "parameters start clean");
    s.set(P_INPUT, 4.0);
    check(s.isDirty(P_INPUT), "UI changes mark a parameter dirty");
    check(s.takeDirty(P_INPUT), "dirty parameters are reported once");
    check(!s.isDirty(P_INPUT), "dirty flag clears after reporting");

    s.setFromHost(P_INPUT, -4.0);
    check(!s.isDirty(P_INPUT), "host changes do not echo back as dirty");

    s.set(P_SLOT0_TYPE, 2.0);
    const Controls c = controlsFrom(s);
    check(c.slot[0].type == 2, "automation reaches the slot module type");
  }
  {
    Shared s;
    s.setFromHost(P_SLOT0_TYPE, 1.0);
    s.setFromHost(P_SLOT0_EQ_LOW, 0.75);
    s.setFromHost(P_SLOT1_TYPE, 2.0);
    s.setFromHost(P_SLOT1_COMP_THRESHOLD, -12.0);
    s.setFromHost(P_SLOT1_COMP_RATIO, 6.0);
    s.setFromHost(P_INPUT, 2.5);
    const Controls c = controlsFrom(s);
    check(c.slot[0].type == 1 && std::fabs(c.slot[0].eqLow - 0.75f) < 1e-6,
          "slot 0 EQ automation maps to the module");
    check(c.slot[1].type == 2 &&
              std::fabs(c.slot[1].compThresholdDb + 12.0f) < 1e-6 &&
              std::fabs(c.slot[1].compRatio - 6.0f) < 1e-6,
          "slot 1 compressor automation maps to the module");
    check(std::fabs(c.inputDb - 2.5f) < 1e-6,
          "global input automation maps to the engine");
  }

  std::printf("%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
