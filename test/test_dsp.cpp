// Suite Rack — DSP tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
#include "dsp/engine.hpp"
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
  for (float v : x) if (!std::isfinite(v)) return false;
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
    c.slot[0].type = 0;
    c.slot[1].type = 0;
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    double err = 0.0;
    for (int i = 0; i < n; ++i) err = std::max(err, static_cast<double>(std::fabs(l[i] - tone[i])));
    check(err < 1e-6, "two empty slots are transparent");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = 1;
    c.slot[0].eqHpf = 1.0f;
    c.slot[1].type = 0;
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
    c.slot[0].type = 2;
    c.slot[0].compThresholdDb = -10.0f;
    c.slot[0].compRatio = 6.0f;
    c.slot[0].compMakeup = 0.4f;
    c.slot[1].type = 0;
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
    c.slot[0].type = 1;
    c.slot[0].eqHpf = 1.0f;
    c.slot[0].bypass = true;
    c.slot[1].type = 0;
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    double err = 0.0;
    for (int i = 0; i < n; ++i) err = std::max(err, static_cast<double>(std::fabs(l[i] - tone[i])));
    check(err < 1e-6, "slot bypass is transparent");
  }
  std::printf("%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
