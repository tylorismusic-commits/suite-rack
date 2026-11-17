// Suite Rack — panel.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/panel.hpp"

#include "presets.hpp"
#include "value_text.hpp"

#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace rack {
namespace {

constexpr double kPi = 3.14159265358979323846;
struct Rgb { double r, g, b; };
const Rgb kAccent{0.93, 0.55, 0.18};
const Rgb kText{0.94, 0.90, 0.84};
const Rgb kDim{0.62, 0.56, 0.49};
const Rgb kEdge{0.34, 0.29, 0.24};

void roundRect(cairo_t* cr, double x, double y, double w, double h, double r) {
  cairo_new_sub_path(cr);
  cairo_arc(cr, x + w - r, y + r, r, -kPi / 2, 0);
  cairo_arc(cr, x + w - r, y + h - r, r, 0, kPi / 2);
  cairo_arc(cr, x + r, y + h - r, r, kPi / 2, kPi);
  cairo_arc(cr, x + r, y + r, r, kPi, 3 * kPi / 2);
  cairo_close_path(cr);
}
void setRgb(cairo_t* cr, Rgb c, double a = 1.0) {
  cairo_set_source_rgba(cr, c.r, c.g, c.b, a);
}
double knobAngle(double f) { return -2.35 + f * 4.70; }
double clamp01(double x) { return std::clamp(x, 0.0, 1.0); }
const char* moduleName(int type) {
  return type == 1 ? "EQ" : (type == 2 ? "COMP" : "EMPTY");
}
} // namespace

Panel::Panel() = default;
Panel::~Panel() {
  if (font_) cairo_font_face_destroy(font_);
  if (ftFace_) FT_Done_Face(static_cast<FT_Face>(ftFace_));
  if (ft_) FT_Done_FreeType(static_cast<FT_Library>(ft_));
  if (grain_) cairo_surface_destroy(grain_);
}

void Panel::loadFont(const std::string& dir) {
  if (ft_) return;
  std::string path = dir;
  if (!path.empty() && path.back() != '/') path += '/';
  path += "Jost-ExtraBold.ttf";
  FT_Library lib = nullptr;
  if (FT_Init_FreeType(&lib) != 0) return;
  FT_Face face = nullptr;
  if (FT_New_Face(lib, path.c_str(), 0, &face) != 0) {
    FT_Done_FreeType(lib);
    return;
  }
  font_ = cairo_ft_font_face_create_for_ft_face(face, 0);
  ft_ = lib;
  ftFace_ = face;
}

void Panel::text(cairo_t* cr, const char* s, double x, double y, double size,
                 int align, double r, double g, double b) const {
  cairo_set_font_face(cr, font_);
  cairo_set_font_size(cr, size);
  cairo_set_source_rgb(cr, r, g, b);
  cairo_text_extents_t e;
  cairo_text_extents(cr, s, &e);
  double tx = x;
  if (align == 1) tx = x - e.x_advance * 0.5;
  else if (align == 2) tx = x - e.x_advance;
  cairo_move_to(cr, tx, y);
  cairo_show_text(cr, s);
}

void Panel::tick(const Shared& shared, PanelState& state) const {
  state.inLevelL = shared.inPeakLDb.load(std::memory_order_relaxed);
  state.inLevelR = shared.inPeakRDb.load(std::memory_order_relaxed);
  state.outLevelL = shared.outPeakLDb.load(std::memory_order_relaxed);
  state.outLevelR = shared.outPeakRDb.load(std::memory_order_relaxed);
  for (int i = 0; i < kSlotCount; ++i)
    state.slotGrDb[i] = shared.slotGrDb[i].load(std::memory_order_relaxed);
}

void Panel::layout(const Shared& shared, const PanelState& state) {
  widgets_.clear();
  Widget p{};
  p.kind = Kind::Power; p.param = P_POWER; p.x = 34; p.y = 32; p.r = 14;
  widgets_.push_back(p);

  auto knob = [&](uint32_t param, double x, double y, double r,
                  const char* label, bool below = false, int steps = 0,
                  std::vector<const char*> scale = {}, bool compact = false) {
    Widget k{};
    k.kind = Kind::Knob; k.param = param; k.x = x; k.y = y; k.r = r;
    k.label = label; k.labelBelow = below; k.steps = steps;
    k.scale = std::move(scale); k.compact = compact;
    widgets_.push_back(k);
  };

  knob(P_INPUT, 130, 665, 18, "INPUT");
  knob(P_OUTPUT, 900, 665, 18, "OUTPUT");

  for (int i = 0; i < kSlotCount; ++i) {
    const double y = 92.0 + i * 88.0;
    Widget name{};
    name.kind = Kind::ModuleName;
    name.param = static_cast<uint32_t>(i);
    name.value = i;
    name.x = 76.0; name.y = y + 30.0; name.w = 190.0; name.h = 28.0;
    widgets_.push_back(name);

    Widget byp{};
    byp.kind = Kind::Pill;
    byp.param = slotParam(i, kSlotOffsetBypass);
    byp.x = 282.0; byp.y = y + 10.0; byp.w = 72.0; byp.h = 24.0;
    byp.label = "BYPASS";
    widgets_.push_back(byp);

    knob(slotParam(i, kSlotOffsetMix), 382.0, y + 52.0, 16.0, "MIX", false, 0, {}, true);

    const int type = static_cast<int>(shared.get(slotParam(i, kSlotOffsetType)) + 0.5);
    if (type == 1) {
      static const char* labels[6] = {"LOW", "LOW MID", "HIGH MID", "HIGH", "HPF", "LPF"};
      for (int k = 0; k < 6; ++k) {
        knob(slotParam(i, kSlotOffsetEqLow + k), 475.0 + k * 82.0,
             y + 44.0, 15.0, labels[k], false, 0, {}, true);
      }
    } else if (type == 2) {
      for (int m = 0; m < 3; ++m) {
        Widget b{};
        b.kind = Kind::ModelButton;
        b.param = slotParam(i, kSlotOffsetCompModel);
        b.value = m;
        b.x = 465.0 + m * 40.0;
        b.y = y + 10.0;
        b.w = 36.0; b.h = 22.0;
        b.label = kCompModelLabels[m];
        widgets_.push_back(b);
      }
      static const char* labels[7] = {"THR", "RAT", "ATK", "REL", "MAKE", "MIX", "SC"};
      for (int k = 0; k < 7; ++k) {
        knob(slotParam(i, kSlotOffsetCompThreshold + k), 600.0 + k * 58.0,
             y + 44.0, 13.0, labels[k], false, 0, {}, true);
      }
    }
  }

  if (state.browserSlot >= 0 && state.browserSlot < kSlotCount) {
    const double bx = 300, by = 220, bw = 440, bh = 250;
    for (int i = 0; i < 3; ++i) {
      Widget b{};
      b.kind = Kind::BrowserOption;
      b.param = slotParam(state.browserSlot, kSlotOffsetType);
      b.value = i;
      b.x = bx + 24 + (i % 2) * 200.0;
      b.y = by + 70 + (i / 2) * 70.0;
      b.w = 180.0; b.h = 48.0;
      b.label = i == 0 ? "OFF" : (i == 1 ? "EQ" : "COMP");
      widgets_.push_back(b);
    }
    Widget cancel{};
    cancel.kind = Kind::BrowserOption;
    cancel.param = slotParam(state.browserSlot, kSlotOffsetType);
    cancel.value = -1;
    cancel.x = bx + 130; cancel.y = by + bh - 46; cancel.w = 180; cancel.h = 30;
    cancel.label = "CANCEL";
    widgets_.push_back(cancel);
  }
}

void Panel::drawHeader(cairo_t* cr, const Shared& shared, const PanelState& state) const {
  cairo_set_source_rgb(cr, 0.055, 0.052, 0.048);
  cairo_rectangle(cr, 0, 0, kPanelWidth, 78);
  cairo_fill(cr);
  setRgb(cr, kEdge, 0.7);
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, 0, 77.5); cairo_line_to(cr, kPanelWidth, 77.5); cairo_stroke(cr);

  const bool on = shared.get(P_POWER) > 0.5;
  cairo_arc(cr, 34, 32, 14, 0, 2 * kPi);
  setRgb(cr, {0.09, 0.08, 0.07}); cairo_fill(cr);
  cairo_arc(cr, 34, 32, 11, 0, 2 * kPi);
  setRgb(cr, on ? Rgb{0.38, 0.86, 0.46} : Rgb{0.42, 0.40, 0.38});
  cairo_set_line_width(cr, 2.2); cairo_stroke(cr);
  cairo_move_to(cr, 34, 23); cairo_line_to(cr, 34, 31); cairo_stroke(cr);

  // Comp Suite-style preset bar.
  roundRect(cr, 72, 14, 330, 36, 4);
  setRgb(cr, {0.15, 0.13, 0.11}); cairo_fill(cr);
  setRgb(cr, kEdge, 0.8); cairo_set_line_width(cr, 1.0);
  roundRect(cr, 72.5, 14.5, 329, 35, 4); cairo_stroke(cr);
  text(cr, "<", 90, 39, 16, 1, kText.r, kText.g, kText.b);
  text(cr, ">", 384, 39, 16, 1, kText.r, kText.g, kText.b);
  const char* preset = state.presetIndex >= 0 &&
                               state.presetIndex < kPresetCount
                           ? kPresets[state.presetIndex].name
                           : "Init";
  text(cr, preset, 237, 39, 13, 1, kText.r, kText.g, kText.b);

  text(cr, "SUITE RACK", 520, 44, 24, 1, kText.r, kText.g, kText.b);

  struct Pill { const char* label; double x; };
  const Pill pills[] = {{"6 SLOTS", 650}, {"SERIAL", 760}, {"M0", 880}};
  for (const Pill& pill : pills) {
    roundRect(cr, pill.x, 20, 96, 28, 4);
    setRgb(cr, {0.20, 0.17, 0.14}); cairo_fill(cr);
    text(cr, pill.label, pill.x + 48, 39, 9, 1, kDim.r, kDim.g, kDim.b);
  }
}

void Panel::drawRack(cairo_t* cr, const Shared& shared,
                     const PanelState& state) const {
  for (int i = 0; i < kSlotCount; ++i) {
    const double y = 92.0 + i * 88.0;
    const int type = static_cast<int>(shared.get(slotParam(i, kSlotOffsetType)) + 0.5);
    const bool bypass = shared.get(slotParam(i, kSlotOffsetBypass)) > 0.5;
    roundRect(cr, 60, y, 920, 80, 5);
    setRgb(cr, (i & 1) ? Rgb{0.115, 0.105, 0.095} : Rgb{0.135, 0.122, 0.108});
    cairo_fill(cr);
    cairo_set_line_width(cr, 1.0);
    setRgb(cr, state.browserSlot == i ? kAccent : kEdge,
           state.browserSlot == i ? 0.9 : 0.5);
    roundRect(cr, 60.5, y + 0.5, 919, 79, 5); cairo_stroke(cr);

    char num[16];
    std::snprintf(num, sizeof(num), "%d", i + 1);
    text(cr, num, 76, y + 24, 12, 0, kDim.r, kDim.g, kDim.b);
    if (type == 2) {
      char gr[32];
      std::snprintf(gr, sizeof(gr), "GR %.1f dB", state.slotGrDb[i]);
      text(cr, gr, 250, y + 66, 8, 0, kDim.r, kDim.g, kDim.b);
    }
    text(cr, "MODULE", 76, y + 70, 7, 0, kDim.r, kDim.g, kDim.b);
  }
  text(cr, "VIRTUAL MIX RACK", 520, 84, 9, 1, kDim.r, kDim.g, kDim.b);
}

void Panel::drawBrowser(cairo_t* cr, const Shared& shared,
                        const PanelState& state) const {
  (void)shared;
  if (state.browserSlot < 0 || state.browserSlot >= kSlotCount) return;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.62);
  cairo_paint(cr);
  const double bx = 300, by = 220, bw = 440, bh = 250;
  roundRect(cr, bx, by, bw, bh, 6);
  setRgb(cr, {0.13, 0.115, 0.10}); cairo_fill(cr);
  setRgb(cr, kAccent, 0.85); cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 6); cairo_stroke(cr);
  char title[48];
  std::snprintf(title, sizeof(title), "CHOOSE MODULE · SLOT %d",
                state.browserSlot + 1);
  text(cr, title, 520, by + 38, 14, 1, kText.r, kText.g, kText.b);
  text(cr, "EQ and Comp are available in M0. More modules follow.", 520,
       by + 58, 8.5, 1, kDim.r, kDim.g, kDim.b);
}

void Panel::draw(cairo_t* cr, Shared& shared, const PanelState& state) {
  layout(shared, state);
  cairo_set_source_rgb(cr, 0.07, 0.065, 0.06);
  cairo_paint(cr);
  drawHeader(cr, shared, state);
  drawRack(cr, shared, state);
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::BrowserOption) drawWidget(cr, w, shared, state);
  }
  drawBrowser(cr, shared, state);
  for (const Widget& w : widgets_) {
    if (w.kind == Kind::BrowserOption) drawWidget(cr, w, shared, state);
  }
  drawLevelMeter(cr, 20, 100, 14, 520, state.inLevelL, state.inLevelR);
  drawLevelMeter(cr, 1006, 100, 14, 520, state.outLevelL, state.outLevelR);
  text(cr, "IN", 27, 640, 8, 1, kDim.r, kDim.g, kDim.b);
  text(cr, "OUT", 1013, 640, 8, 1, kDim.r, kDim.g, kDim.b);
  text(cr, "SUITE RACK · VIRTUAL MIX RACK · SIX SLOTS", 520, 712, 11, 1,
       kText.r, kText.g, kText.b);
}

void Panel::drawWidget(cairo_t* cr, const Widget& w, const Shared& shared,
                       const PanelState&) const {
  switch (w.kind) {
    case Kind::Power: {
      const bool on = shared.get(P_POWER) > 0.5;
      cairo_arc(cr, w.x, w.y, w.r, 0, 2 * kPi);
      setRgb(cr, {0.10, 0.09, 0.08}); cairo_fill(cr);
      cairo_arc(cr, w.x, w.y, w.r - 1.5, 0, 2 * kPi);
      setRgb(cr, on ? Rgb{0.38, 0.86, 0.46} : Rgb{0.42, 0.40, 0.38});
      cairo_set_line_width(cr, 2.2);
      cairo_arc(cr, w.x, w.y, 7, -kPi / 2 + 0.6, -kPi / 2 + 2 * kPi - 0.6);
      cairo_stroke(cr);
      cairo_move_to(cr, w.x, w.y - 9); cairo_line_to(cr, w.x, w.y - 1);
      cairo_stroke(cr);
      break;
    }
    case Kind::SlotCard:
      break;
    case Kind::ModuleName: {
      const int slot = static_cast<int>(w.value);
      const int type = static_cast<int>(
          shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, {0.22, 0.19, 0.16}); cairo_fill(cr);
      text(cr, moduleName(type), w.x + 12, w.y + w.h * 0.5 + 5, 12, 0,
           kText.r, kText.g, kText.b);
      text(cr, "CHOOSE", w.x + w.w - 10, w.y + w.h * 0.5 + 4, 7, 2,
           kDim.r, kDim.g, kDim.b);
      break;
    }
    case Kind::BrowserOption: {
      const bool cancel = w.value < 0;
      const bool current = !cancel &&
          std::fabs(shared.get(w.param) - w.value) < 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, cancel ? Rgb{0.22, 0.19, 0.16}
                        : (current ? kAccent : Rgb{0.28, 0.24, 0.20}));
      cairo_fill(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 5,
           cancel ? 9 : 14, 1,
           current ? 0.10 : kText.r, current ? 0.08 : kText.g,
           current ? 0.05 : kText.b);
      break;
    }
    case Kind::ModuleButton: {
      const bool lit = std::fabs(shared.get(w.param) - w.value) < 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, lit ? kAccent : Rgb{0.22, 0.19, 0.16}); cairo_fill(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
           lit ? 0.10 : 0.78, lit ? 0.08 : 0.74, lit ? 0.05 : 0.68);
      break;
    }
    case Kind::ModelButton: {
      const bool lit = std::fabs(shared.get(w.param) - w.value) < 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, lit ? kAccent : Rgb{0.22, 0.19, 0.16}); cairo_fill(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
           lit ? 0.10 : 0.78, lit ? 0.08 : 0.74, lit ? 0.05 : 0.68);
      break;
    }
    case Kind::Pill: {
      const bool on = shared.get(w.param) > 0.5;
      drawPill(cr, w, on, w.label);
      break;
    }
    case Kind::Button:
      drawButton(cr, w, false);
      break;
    case Kind::Knob:
      drawKnob(cr, w, shared.get(w.param));
      break;
  }
}

void Panel::drawKnob(cairo_t* cr, const Widget& w, double value) const {
  const ParamSpec& s = kParamSpecs[w.param];
  const double vmax = w.steps > 1 ? w.steps - 1 : s.max;
  const double span = vmax - s.min;
  const double f = clamp01(span > 0.0 ? (value - s.min) / span : 0.0);
  const double a = knobAngle(f);
  setRgb(cr, kDim, 0.55);
  cairo_set_line_width(cr, 1.0);
  for (int i = 0; i <= 10; ++i) {
    const double aa = knobAngle(i / 10.0);
    cairo_move_to(cr, w.x + std::cos(aa) * w.r * 1.10,
                  w.y + std::sin(aa) * w.r * 1.10);
    cairo_line_to(cr, w.x + std::cos(aa) * w.r * 1.20,
                  w.y + std::sin(aa) * w.r * 1.20);
    cairo_stroke(cr);
  }
  cairo_arc(cr, w.x, w.y, w.r, 0, 2 * kPi);
  setRgb(cr, {0.20, 0.18, 0.16}); cairo_fill(cr);
  cairo_arc(cr, w.x, w.y, w.r * 0.82, 0, 2 * kPi);
  setRgb(cr, {0.86, 0.82, 0.74}); cairo_fill(cr);
  cairo_set_line_width(cr, 2.0);
  setRgb(cr, kAccent);
  cairo_arc(cr, w.x, w.y, w.r + 2, knobAngle(0), a); cairo_stroke(cr);
  cairo_set_line_width(cr, 2.4);
  setRgb(cr, kText);
  cairo_move_to(cr, w.x + std::cos(a) * w.r * 0.2, w.y + std::sin(a) * w.r * 0.2);
  cairo_line_to(cr, w.x + std::cos(a) * w.r * 0.65, w.y + std::sin(a) * w.r * 0.65);
  cairo_stroke(cr);
  const double labelY = w.labelBelow ? w.y + w.r * 1.5 + 20
                                     : w.y - w.r * 1.5 - (w.compact ? 6.0 : 14.0);
  text(cr, w.label, w.x, labelY, w.compact ? 7.0 : 10.0, 1,
       kText.r, kText.g, kText.b);
  const std::string val = formatParamValue(Shared{}, w.param, value);
  text(cr, val.c_str(), w.x, w.y + w.r + (w.compact ? 8.0 : 14.0),
       w.compact ? 6.5 : 8.5, 1, kAccent.r, kAccent.g, kAccent.b);
}

void Panel::drawPill(cairo_t* cr, const Widget& w, bool on, const char* label) const {
  roundRect(cr, w.x, w.y, w.w, w.h, 4);
  setRgb(cr, on ? kAccent : Rgb{0.22, 0.19, 0.16}); cairo_fill(cr);
  text(cr, label ? label : w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
       on ? 0.10 : 0.78, on ? 0.08 : 0.74, on ? 0.05 : 0.68);
}
void Panel::drawButton(cairo_t* cr, const Widget& w, bool lit) const {
  roundRect(cr, w.x, w.y, w.w, w.h, 4);
  setRgb(cr, lit ? kAccent : Rgb{0.22, 0.19, 0.16}); cairo_fill(cr);
  text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
       lit ? 0.10 : 0.78, lit ? 0.08 : 0.74, lit ? 0.05 : 0.68);
}
void Panel::drawLevelMeter(cairo_t* cr, double x, double y, double w, double h,
                           double dbL, double dbR) const {
  roundRect(cr, x, y, w, h, 3); setRgb(cr, {0.055, 0.052, 0.048}); cairo_fill(cr);
  const double half = w * 0.5;
  for (int ch = 0; ch < 2; ++ch) {
    const double db = ch == 0 ? dbL : dbR;
    const double f = clamp01((db + 60.0) / 60.0);
    const double bx = x + 2 + ch * (half - 2);
    setRgb(cr, f > 0.92 ? Rgb{0.95, 0.25, 0.18}
                       : (f > 0.78 ? kAccent : Rgb{0.36, 0.78, 0.40}), 0.9);
    cairo_rectangle(cr, bx, y + h - 3 - (h - 6) * f, half - 4, (h - 6) * f);
    cairo_fill(cr);
  }
}

bool Panel::hitWidget(const Widget& w, double x, double y) const {
  if (w.kind == Kind::Knob) {
    const double dx = x - w.x, dy = y - w.y;
    return dx * dx + dy * dy <= w.r * w.r * 1.5;
  }
  if (w.kind == Kind::Power) {
    const double dx = x - w.x, dy = y - w.y;
    return dx * dx + dy * dy <= w.r * w.r * 1.4;
  }
  return x >= w.x && x <= w.x + w.w && y >= w.y && y <= w.y + w.h;
}

bool Panel::pointerDown(Shared& shared, PanelState& state, double x, double y,
                        bool fine) {
  (void)fine;
  if (handlePresetClick(shared, state, x, y)) return true;
  if (state.browserSlot >= 0) {
    for (const Widget& w : widgets_) {
      if (w.kind != Kind::BrowserOption || !hitWidget(w, x, y)) continue;
      if (w.value >= 0) shared.set(w.param, w.value);
      state.browserSlot = -1;
      state.presetDirty = true;
      return true;
    }
    state.browserSlot = -1;
    return true;
  }
  for (const Widget& w : widgets_) {
    if (!hitWidget(w, x, y)) continue;
    switch (w.kind) {
      case Kind::Power:
        shared.set(P_POWER, shared.get(P_POWER) > 0.5 ? 0.0 : 1.0);
        state.presetDirty = true;
        return true;
      case Kind::SlotCard:
        state.selectedSlot = static_cast<int>(w.param);
        return true;
      case Kind::ModuleName:
        state.selectedSlot = static_cast<int>(w.value);
        state.browserSlot = static_cast<int>(w.value);
        return true;
      case Kind::BrowserOption:
        break;
      case Kind::ModuleButton:
        shared.set(w.param, w.value);
        state.presetDirty = true;
        return true;
      case Kind::ModelButton:
        shared.set(w.param, w.value);
        state.presetDirty = true;
        return true;
      case Kind::Pill:
        shared.set(w.param, shared.get(w.param) > 0.5 ? 0.0 : 1.0);
        state.presetDirty = true;
        return true;
      case Kind::Button:
        shared.set(w.param, w.value);
        state.presetDirty = true;
        return true;
      case Kind::Knob:
        dragParam_ = static_cast<int>(w.param);
        dragSteps_ = w.steps;
        dragStartY_ = y;
        dragStartValue_ = shared.get(w.param);
        if (dragSteps_ > 1) dragStartValue_ = std::min(dragStartValue_, dragSteps_ - 1.0);
        return false;
    }
  }
  return false;
}

bool Panel::pointerMove(Shared& shared, PanelState& state, double, double y,
                        bool fine) {
  if (dragParam_ < 0) return false;
  const uint32_t param = static_cast<uint32_t>(dragParam_);
  const ParamSpec& spec = kParamSpecs[param];
  const double perPixel = spec.stepped ? 1.0 / 18.0
                                       : (spec.max - spec.min) / (fine ? 800.0 : 200.0);
  double v = dragStartValue_ + (dragStartY_ - y) * perPixel;
  if (dragSteps_ > 1) v = std::min(v, dragSteps_ - 1.0);
  shared.set(param, v);
  state.presetDirty = true;
  return true;
}

void Panel::pointerUp() { dragParam_ = -1; }

bool Panel::wheel(Shared& shared, PanelState& state, double x, double y, double dy) {
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::Knob || !hitWidget(w, x, y)) continue;
    const ParamSpec& spec = kParamSpecs[w.param];
    if (spec.stepped)
      shared.set(w.param, shared.get(w.param) - (dy > 0 ? 1.0 : -1.0));
    else
      shared.set(w.param, shared.get(w.param) - dy * (spec.max - spec.min) * 0.01);
    state.presetDirty = true;
    return true;
  }
  return false;
}

bool Panel::handlePresetClick(Shared& shared, PanelState& state, double x,
                              double y) {
  if (y < 14 || y > 50) return false;
  if (x >= 72 && x <= 108) {
    const int next = state.presetIndex <= 0 ? 0 : state.presetIndex - 1;
    applyPreset(shared, state, next);
    return true;
  }
  if (x >= 366 && x <= 402) {
    const int next = std::min(kPresetCount - 1, state.presetIndex + 1);
    applyPreset(shared, state, next);
    return true;
  }
  return x >= 72 && x <= 402;
}

void Panel::applyPreset(Shared& shared, PanelState& state, int index) const {
  if (index < 0 || index >= kPresetCount) return;
  const Preset& p = kPresets[index];
  for (int i = 0; i < p.count; ++i) {
    const PresetValue& v = p.values[i];
    if (v.param >= 0 && v.param < static_cast<int>(P_COUNT))
      shared.set(static_cast<uint32_t>(v.param), v.value);
  }
  state.presetIndex = index;
  state.presetDirty = false;
}

void Panel::drawPresetBar(cairo_t*, const Shared&, const PanelState&) const {}
void Panel::drawPresetList(cairo_t*, const Shared&, const PanelState&) const {}
void Panel::beginTextEntry(Shared&, PanelState&, double, double) {}
bool Panel::textKey(Shared&, PanelState&, int, char) { return false; }

std::vector<Panel::WidgetRect> Panel::widgetRects() const {
  std::vector<WidgetRect> out;
  for (const Widget& w : widgets_) {
    WidgetRect r;
    r.param = w.param;
    r.label = w.label;
    r.knob = w.kind == Kind::Knob;
    r.toggle = w.kind == Kind::Pill;
    r.r = w.r;
    if (w.kind == Kind::Knob) {
      r.x = w.x - w.r; r.y = w.y - w.r; r.w = w.r * 2; r.h = w.r * 2;
    } else {
      r.x = w.x; r.y = w.y; r.w = w.w; r.h = w.h;
    }
    out.push_back(r);
  }
  return out;
}

} // namespace rack
