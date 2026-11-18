// Suite Rack — DSP tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include "dsp/engine.hpp"
#include "module_ids.hpp"
#include "params.hpp"

using namespace rack;

namespace {
int fails = 0;
void check(bool ok, const char* what) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) ++fails;
}
double rms(const std::vector<float>& x, size_t a, size_t b) {
  double s = 0.0;
  for (size_t i = a; i < b; ++i) s += double(x[i]) * x[i];
  return std::sqrt(s / std::max<size_t>(1, b - a));
}
bool finite(const std::vector<float>& x) {
  for (float v : x)
    if (!std::isfinite(v)) return false;
  return true;
}
} // namespace

int main() {
  const float sr = 48000.0f;
  const int n = static_cast<int>(sr * 0.5f);
  std::vector<float> tone(n);
  for (int i = 0; i < n; ++i)
    tone[i] = 0.25f * std::sin(2.0f * 3.14159265f * 1000.0f * i / sr);

  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Off);
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    double err = 0.0;
    for (int i = 0; i < n; ++i)
      err = std::max(err, static_cast<double>(std::fabs(l[i] - tone[i])));
    check(err < 1e-6, "two empty slots are transparent");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Eq4K);
    c.slot[0].c[kCtlHpf] = 1.0f;
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    std::vector<float> low(n);
    for (int i = 0; i < n; ++i)
      low[i] = 0.25f * std::sin(2.0f * 3.14159265f * 100.0f * i / sr);
    auto l = low;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    check(rms(l, 1000, n) < rms(low, 1000, n) * 0.5,
          "EQ slot high-pass removes a 100 Hz tone at maximum HPF");
    check(finite(l), "EQ slot stays finite");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Comp76Blue);
    c.slot[0].c[kCtlThreshold] = 0.75f;
    c.slot[0].c[kCtlRatio] = 0.75f;
    c.slot[0].c[kCtlMakeup] = 0.40f;
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    check(rms(l, 2000, n) < rms(tone, 2000, n) * 0.95,
          "compressor slot reduces a loud tone");
    check(m.slotGrDb[0] > 0.5f, "compressor slot reports gain reduction");
    check(finite(l), "compressor slot stays finite");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::EqHearth);
    c.slot[0].c[kCtlHpf] = 1.0f;
    c.slot[0].bypass = true;
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    double err = 0.0;
    for (int i = 0; i < n; ++i)
      err = std::max(err, static_cast<double>(std::fabs(l[i] - tone[i])));
    check(err < 1e-6, "slot bypass is transparent");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Eq4K);
    c.slot[0].gainDb = -6.0f;
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    check(rms(l, 2000, n) < rms(tone, 2000, n) * 0.6,
          "per-slot gain trims the module output");
  }

  {
    const int ids[] = {
        static_cast<int>(ModuleId::EqHearth),
        static_cast<int>(ModuleId::EqTilt),
        static_cast<int>(ModuleId::EqAir),
        static_cast<int>(ModuleId::Comp76Black),
        static_cast<int>(ModuleId::Comp2A),
        static_cast<int>(ModuleId::Comp3A),
        static_cast<int>(ModuleId::CompVCA),
        static_cast<int>(ModuleId::ColourValve),
        static_cast<int>(ModuleId::ColourConsole),
        static_cast<int>(ModuleId::ColourBus),
        static_cast<int>(ModuleId::ColourTape),
        static_cast<int>(ModuleId::ColourTransformer),
        static_cast<int>(ModuleId::DynGate),
        static_cast<int>(ModuleId::DynTransient),
        static_cast<int>(ModuleId::DynClip),
        static_cast<int>(ModuleId::DynDeEss),
    };
    for (int id : ids) {
      Engine e;
      e.prepare(sr);
      Controls c;
      c.slot[0].type = id;
      for (int k = 0; k < kRackControlCount; ++k) {
        c.slot[0].c[k] = static_cast<float>(
            defaultControlValue(static_cast<ModuleId>(id), k));
      }
      if (moduleIsComp(id)) {
        c.slot[0].c[kCtlThreshold] = 0.80f;
        c.slot[0].c[kCtlRatio] = 0.75f;
        c.slot[0].c[kCtlMix] = 1.0f;
      }
      c.slot[1].type = static_cast<int>(ModuleId::Off);
      e.setControls(c);
      auto l = tone;
      Meters m;
      e.process(l.data(), l.data(), n, m);
      check(finite(l), kModuleLabels[id]);
    }
  }
  std::printf("%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
