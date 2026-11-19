// Suite Rack — panel.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/panel.hpp"

#include "factory_module_presets.hpp"
#include "modules.hpp"
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

// Rack preset browser geometry.
constexpr double kPresetListX = 72.0;
constexpr double kPresetListY = 54.0;
constexpr double kPresetListW = 380.0;
constexpr double kPresetRowH = 20.0;
constexpr double kPresetBottomGap = 8.0;
constexpr int kPresetVisibleRows = 12;
constexpr double kPresetHeaderH = 46.0;
constexpr double kPresetFooterH = 34.0;

int presetFilterTypeFor(const Shared& shared, const PanelState& state) {
  if (!state.browserPresetFilter || state.browserSlot < 0) return -1;
  return static_cast<int>(
      shared.get(slotParam(state.browserSlot, kSlotOffsetType)) + 0.5);
}

int presetRowsFor(int count) {
  return std::clamp(count, 3, kPresetVisibleRows);
}

bool cursorVisible() {
  using namespace std::chrono;
  const auto ms = duration_cast<milliseconds>(
                      steady_clock::now().time_since_epoch())
                      .count();
  return (ms / 500) % 2 == 0;
}
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
const char* slotModuleName(int type) {
  return type == static_cast<int>(ModuleId::Off) ? "EMPTY"
                                                 : moduleDesc(type)->name;
}

struct ModuleStyle {
  Rgb body;
  Rgb edge;
  Rgb accent;
  Rgb knob;
};

ModuleStyle styleFor(int type) {
  switch (static_cast<ModuleId>(type)) {
    case ModuleId::Eq4K:
      return {{0.10, 0.145, 0.215}, {0.18, 0.28, 0.40},
              {0.34, 0.74, 0.96}, {0.86, 0.88, 0.90}};
    case ModuleId::EqHearth:
      return {{0.22, 0.115, 0.075}, {0.46, 0.24, 0.14},
              {0.98, 0.66, 0.30}, {0.92, 0.85, 0.72}};
    case ModuleId::EqTilt:
      return {{0.16, 0.17, 0.18}, {0.30, 0.33, 0.35},
              {0.72, 0.78, 0.84}, {0.88, 0.89, 0.90}};
    case ModuleId::EqAir:
      return {{0.10, 0.17, 0.22}, {0.20, 0.36, 0.45},
              {0.56, 0.86, 0.98}, {0.86, 0.91, 0.93}};
    case ModuleId::Comp76Blue:
      return {{0.09, 0.12, 0.20}, {0.18, 0.26, 0.40},
              {0.42, 0.70, 1.00}, {0.84, 0.87, 0.92}};
    case ModuleId::Comp76Black:
      return {{0.075, 0.075, 0.075}, {0.20, 0.20, 0.20},
              {0.78, 0.80, 0.84}, {0.82, 0.82, 0.82}};
    case ModuleId::Comp2A:
      return {{0.17, 0.17, 0.16}, {0.34, 0.34, 0.32},
              {0.80, 0.82, 0.76}, {0.90, 0.90, 0.86}};
    case ModuleId::Comp3A:
      return {{0.16, 0.18, 0.16}, {0.32, 0.36, 0.31},
              {0.78, 0.86, 0.68}, {0.88, 0.91, 0.84}};
    case ModuleId::CompVCA:
      return {{0.10, 0.18, 0.13}, {0.20, 0.36, 0.26},
              {0.52, 0.90, 0.60}, {0.86, 0.92, 0.87}};
    case ModuleId::ColourValve:
      return {{0.22, 0.15, 0.08}, {0.48, 0.32, 0.14},
              {0.98, 0.72, 0.30}, {0.94, 0.86, 0.68}};
    case ModuleId::ColourConsole:
      return {{0.25, 0.16, 0.09}, {0.52, 0.30, 0.14},
              {0.98, 0.62, 0.24}, {0.95, 0.88, 0.74}};
    case ModuleId::ColourBus:
      return {{0.26, 0.10, 0.09}, {0.54, 0.20, 0.16},
              {0.98, 0.42, 0.28}, {0.95, 0.84, 0.76}};
    case ModuleId::ColourTape:
      return {{0.24, 0.20, 0.14}, {0.48, 0.40, 0.28},
              {0.92, 0.80, 0.58}, {0.96, 0.90, 0.78}};
    case ModuleId::ColourTransformer:
      return {{0.20, 0.14, 0.10}, {0.44, 0.30, 0.18},
              {0.86, 0.56, 0.30}, {0.94, 0.84, 0.70}};
    case ModuleId::DynGate:
      return {{0.16, 0.10, 0.20}, {0.34, 0.20, 0.42},
              {0.78, 0.52, 0.96}, {0.92, 0.86, 0.94}};
    case ModuleId::DynTransient:
      return {{0.24, 0.14, 0.07}, {0.50, 0.30, 0.12},
              {0.98, 0.64, 0.26}, {0.95, 0.87, 0.72}};
    case ModuleId::DynClip:
      return {{0.26, 0.08, 0.07}, {0.54, 0.18, 0.14},
              {0.98, 0.34, 0.24}, {0.95, 0.82, 0.74}};
    case ModuleId::DynDeEss:
      return {{0.23, 0.12, 0.18}, {0.48, 0.24, 0.34},
              {0.98, 0.54, 0.72}, {0.95, 0.84, 0.88}};
    case ModuleId::Density:
      return {{0.08, 0.075, 0.07}, {0.24, 0.20, 0.16},
              {0.92, 0.72, 0.34}, {0.90, 0.84, 0.72}};
    case ModuleId::BleedOut:
      return {{0.17, 0.09, 0.21}, {0.38, 0.18, 0.46},
              {0.84, 0.48, 0.98}, {0.94, 0.86, 0.96}};
    case ModuleId::Limiter:
      return {{0.06, 0.06, 0.055}, {0.22, 0.21, 0.18},
              {0.96, 0.78, 0.36}, {0.90, 0.86, 0.74}};
    case ModuleId::Image:
      return {{0.09, 0.17, 0.15}, {0.18, 0.34, 0.30},
              {0.48, 0.92, 0.78}, {0.86, 0.93, 0.90}};
    case ModuleId::Crush:
      return {{0.24, 0.11, 0.07}, {0.50, 0.24, 0.12},
              {0.98, 0.56, 0.24}, {0.94, 0.86, 0.72}};
    case ModuleId::Filter:
      return {{0.08, 0.14, 0.21}, {0.18, 0.30, 0.42},
              {0.42, 0.76, 0.98}, {0.86, 0.91, 0.94}};
    case ModuleId::Motion:
      return {{0.24, 0.13, 0.08}, {0.50, 0.28, 0.14},
              {0.98, 0.66, 0.28}, {0.95, 0.88, 0.74}};
    case ModuleId::Sub:
      return {{0.13, 0.09, 0.21}, {0.30, 0.18, 0.46},
              {0.68, 0.48, 0.98}, {0.90, 0.86, 0.96}};
    case ModuleId::Double:
      return {{0.08, 0.17, 0.18}, {0.18, 0.36, 0.38},
              {0.44, 0.88, 0.86}, {0.86, 0.93, 0.92}};
    case ModuleId::DynamicEq:
      return {{0.23, 0.14, 0.07}, {0.48, 0.30, 0.13},
              {0.98, 0.66, 0.26}, {0.94, 0.87, 0.72}};
    case ModuleId::Multiband:
      return {{0.08, 0.12, 0.23}, {0.18, 0.26, 0.48},
              {0.48, 0.66, 0.98}, {0.86, 0.90, 0.96}};
    case ModuleId::Rider:
      return {{0.10, 0.18, 0.12}, {0.20, 0.36, 0.24},
              {0.52, 0.92, 0.58}, {0.86, 0.93, 0.87}};
    case ModuleId::Cleanup:
      return {{0.14, 0.14, 0.13}, {0.30, 0.30, 0.28},
              {0.78, 0.80, 0.76}, {0.89, 0.89, 0.86}};
    case ModuleId::SpacePlate:
      return {{0.12, 0.10, 0.24}, {0.26, 0.22, 0.50},
              {0.64, 0.56, 1.00}, {0.88, 0.86, 0.96}};
    case ModuleId::SpaceRoom:
      return {{0.14, 0.10, 0.22}, {0.30, 0.22, 0.46},
              {0.72, 0.54, 0.98}, {0.90, 0.86, 0.95}};
    case ModuleId::SpaceHall:
      return {{0.09, 0.11, 0.26}, {0.20, 0.24, 0.52},
              {0.52, 0.62, 1.00}, {0.86, 0.88, 0.96}};
    case ModuleId::SpaceSpring:
      return {{0.15, 0.09, 0.20}, {0.32, 0.20, 0.42},
              {0.82, 0.50, 0.94}, {0.92, 0.86, 0.94}};
    case ModuleId::SpaceDigital:
      return {{0.07, 0.17, 0.16}, {0.16, 0.34, 0.32},
              {0.40, 0.90, 0.82}, {0.86, 0.93, 0.91}};
    case ModuleId::SpaceTape:
      return {{0.10, 0.18, 0.11}, {0.22, 0.36, 0.24},
              {0.52, 0.88, 0.60}, {0.86, 0.93, 0.87}};
    case ModuleId::SpaceAnalog:
      return {{0.14, 0.17, 0.09}, {0.30, 0.36, 0.18},
              {0.72, 0.86, 0.42}, {0.90, 0.93, 0.84}};
    case ModuleId::SpacePingPong:
      return {{0.07, 0.14, 0.20}, {0.16, 0.30, 0.42},
              {0.40, 0.76, 0.98}, {0.86, 0.92, 0.95}};
    case ModuleId::SpaceChorus:
      return {{0.24, 0.14, 0.07}, {0.50, 0.30, 0.12},
              {0.98, 0.64, 0.26}, {0.95, 0.87, 0.72}};
    case ModuleId::SpaceFlanger:
      return {{0.24, 0.17, 0.06}, {0.50, 0.36, 0.12},
              {0.98, 0.74, 0.22}, {0.95, 0.90, 0.74}};
    case ModuleId::SpacePhaser:
      return {{0.25, 0.12, 0.10}, {0.52, 0.24, 0.16},
              {0.98, 0.50, 0.30}, {0.95, 0.85, 0.78}};
    case ModuleId::SpaceDimension:
      return {{0.24, 0.10, 0.15}, {0.50, 0.20, 0.28},
              {0.98, 0.44, 0.58}, {0.95, 0.84, 0.88}};
    case ModuleId::HearthComp:
      return {{0.24, 0.10, 0.08}, {0.52, 0.22, 0.14},
              {0.98, 0.50, 0.26}, {0.95, 0.84, 0.76}};
    case ModuleId::Glue:
      return {{0.08, 0.13, 0.22}, {0.18, 0.28, 0.46},
              {0.46, 0.70, 0.98}, {0.86, 0.90, 0.96}};
    case ModuleId::CompXpress:
      return {{0.24, 0.09, 0.07}, {0.52, 0.20, 0.14},
              {0.98, 0.40, 0.26}, {0.95, 0.82, 0.74}};
    case ModuleId::CompDigital:
      return {{0.10, 0.13, 0.18}, {0.22, 0.28, 0.38},
              {0.52, 0.72, 0.94}, {0.86, 0.90, 0.94}};
    case ModuleId::CompMolot:
      return {{0.24, 0.10, 0.10}, {0.52, 0.20, 0.18},
              {0.98, 0.42, 0.34}, {0.95, 0.82, 0.80}};
    case ModuleId::CompRed:
      return {{0.22, 0.06, 0.07}, {0.50, 0.14, 0.14},
              {0.98, 0.30, 0.24}, {0.94, 0.80, 0.76}};
    case ModuleId::Reel:
      return {{0.23, 0.19, 0.13}, {0.47, 0.38, 0.26},
              {0.92, 0.78, 0.54}, {0.96, 0.90, 0.78}};
    case ModuleId::Ember:
      return {{0.09, 0.07, 0.05}, {0.28, 0.20, 0.14},
              {0.98, 0.74, 0.30}, {0.92, 0.86, 0.72}};
    case ModuleId::Tame:
      return {{0.07, 0.17, 0.17}, {0.16, 0.34, 0.34},
              {0.42, 0.90, 0.88}, {0.86, 0.93, 0.92}};
    case ModuleId::Eq510:
      return {{0.24, 0.18, 0.12}, {0.50, 0.36, 0.22},
              {0.98, 0.62, 0.30}, {0.95, 0.88, 0.76}};
    case ModuleId::Pultec:
      return {{0.20, 0.19, 0.16}, {0.42, 0.40, 0.32},
              {0.90, 0.82, 0.64}, {0.95, 0.92, 0.84}};
    case ModuleId::JcClassicLimiter:
      return {{0.08, 0.10, 0.18}, {0.18, 0.22, 0.40},
              {0.50, 0.62, 0.98}, {0.86, 0.89, 0.95}};
    case ModuleId::JcTapeHead:
      return {{0.22, 0.18, 0.12}, {0.46, 0.37, 0.24},
              {0.92, 0.76, 0.50}, {0.95, 0.89, 0.76}};
    case ModuleId::JcTubeDriver:
      return {{0.23, 0.13, 0.07}, {0.48, 0.28, 0.12},
              {0.98, 0.64, 0.26}, {0.95, 0.88, 0.74}};
    case ModuleId::JcDaTube:
      return {{0.24, 0.10, 0.06}, {0.50, 0.22, 0.12},
              {0.98, 0.48, 0.22}, {0.95, 0.84, 0.74}};
    case ModuleId::JcFattener:
      return {{0.22, 0.08, 0.08}, {0.48, 0.18, 0.16},
              {0.98, 0.40, 0.32}, {0.94, 0.82, 0.78}};
    case ModuleId::JcAC1:
      return {{0.13, 0.14, 0.17}, {0.30, 0.32, 0.38},
              {0.64, 0.74, 0.96}, {0.88, 0.90, 0.94}};
    case ModuleId::JcLMC1:
      return {{0.17, 0.17, 0.16}, {0.34, 0.34, 0.32},
              {0.82, 0.82, 0.76}, {0.90, 0.90, 0.86}};
    case ModuleId::JcModernLimiter:
      return {{0.08, 0.11, 0.20}, {0.18, 0.24, 0.44},
              {0.46, 0.66, 1.00}, {0.86, 0.90, 0.96}};
    case ModuleId::JcHyrax:
      return {{0.16, 0.10, 0.08}, {0.36, 0.22, 0.16},
              {0.98, 0.62, 0.34}, {0.94, 0.90, 0.86}};
    case ModuleId::JcBloom:
      return {{0.19, 0.12, 0.20}, {0.34, 0.22, 0.38},
              {0.92, 0.60, 0.98}, {0.94, 0.90, 0.94}};
    case ModuleId::JcRS124:
      return {{0.11, 0.13, 0.12}, {0.24, 0.28, 0.24},
              {0.72, 0.86, 0.62}, {0.90, 0.93, 0.86}};
    case ModuleId::JcCL1B:
      return {{0.10, 0.14, 0.18}, {0.20, 0.30, 0.38},
              {0.58, 0.86, 0.96}, {0.88, 0.94, 0.96}};
    case ModuleId::JcDuTape:
      return {{0.16, 0.13, 0.10}, {0.32, 0.26, 0.19},
              {0.96, 0.78, 0.50}, {0.94, 0.90, 0.82}};
    case ModuleId::JcVBL:
      return {{0.17, 0.10, 0.08}, {0.38, 0.20, 0.14},
              {1.00, 0.60, 0.38}, {0.96, 0.90, 0.84}};
    case ModuleId::JcL2:
      return {{0.08, 0.12, 0.16}, {0.18, 0.28, 0.36},
              {0.44, 0.80, 0.98}, {0.86, 0.94, 0.98}};
    case ModuleId::JcO3:
      return {{0.09, 0.10, 0.17}, {0.20, 0.22, 0.40},
              {0.62, 0.66, 1.00}, {0.88, 0.90, 0.98}};
    case ModuleId::HybridEq:
      return {{0.16, 0.12, 0.09}, {0.33, 0.24, 0.16},
              {0.98, 0.74, 0.40}, {0.95, 0.90, 0.82}};
    case ModuleId::CustomEq:
      return {{0.10, 0.14, 0.16}, {0.20, 0.30, 0.34},
              {0.52, 0.92, 0.86}, {0.88, 0.96, 0.94}};
    default:
      return {{0.105, 0.10, 0.095}, {0.24, 0.22, 0.19},
              {0.55, 0.50, 0.44}, {0.72, 0.70, 0.66}};
  }
}
bool browserCategoryMatches(int id, int category) {
  const ModuleDesc* d = moduleDesc(id);
  if (category == 1) return std::strcmp(d->category, "EQ") == 0;
  if (category == 2) return std::strcmp(d->category, "Comp") == 0;
  if (category == 3) return std::strcmp(d->category, "Colour") == 0;
  if (category == 4) return std::strcmp(d->category, "Dyn") == 0;
  if (category == 5) return std::strcmp(d->category, "Space") == 0;
  return true;
}

bool browserModuleMatchesQuery(int id, const std::string& query) {
  if (query.empty()) return true;
  const ModuleDesc* d = moduleDesc(id);
  std::string hay = std::string(d->name) + " " + d->category;
  hay = lowerText(hay);
  return hay.find(lowerText(query)) != std::string::npos;
}

int browserVisibleCount(const std::string& query) {
  return query.empty() ? 16 : 8;
}

int browserEntryCount(int category, const std::string& query) {
  int count = query.empty() ? 1 : 0;  // Off
  for (int id = 1; id < kModuleCount; ++id) {
    if (!browserCategoryMatches(id, category)) continue;
    if (!browserModuleMatchesQuery(id, query)) continue;
    ++count;
  }
  return count;
}

} // namespace

Panel::Panel() {
  userPresets_ = loadUserPresets(userPresetDir());
  modulePresets_ = loadModulePresets(modulePresetDir());
}
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
  p.kind = Kind::Power; p.param = P_POWER; p.x = 34; p.y = 30; p.r = 14;
  widgets_.push_back(p);

  auto knob = [&](uint32_t param, double x, double y, double r,
                  const char* label, int styleSlot = -1, bool compact = true,
                  int moduleControl = -1, int steps = 0) {
    Widget k{};
    k.kind = Kind::Knob; k.param = param; k.x = x; k.y = y; k.r = r;
    k.label = label; k.labelBelow = false; k.compact = compact;
    k.styleSlot = styleSlot;
    k.styleType = styleSlot >= 0
                      ? static_cast<int>(shared.get(
                            slotParam(styleSlot, kSlotOffsetType)) + 0.5)
                      : -1;
    k.moduleControl = moduleControl;
    k.steps = steps;
    widgets_.push_back(k);
  };

  knob(P_INPUT, 140, 675, 16, "", -1, true);
  knob(P_OUTPUT, 860, 675, 16, "", -1, true);

  const double colW = 143.0;
  const double gap = 12.0;
  for (int i = 0; i < kSlotCount; ++i) {
    const double x = 60.0 + i * (colW + gap);
    const double y = 92.0;
    const int type =
        static_cast<int>(shared.get(slotParam(i, kSlotOffsetType)) + 0.5);

    Widget name{};
    name.kind = Kind::ModuleName;
    name.param = static_cast<uint32_t>(i);
    name.value = i;
    name.x = x + 10; name.y = y + 28; name.w = colW - 20; name.h = 28;
    name.styleSlot = i;
    name.styleType = type;
    widgets_.push_back(name);

    if (type != static_cast<int>(ModuleId::Off)) {
      Widget presets{};
      presets.kind = Kind::ModulePresetButton;
      presets.param = static_cast<uint32_t>(i);
      presets.value = i;
      presets.x = x + colW - 64;
      presets.y = y + 8;
      presets.w = 56;
      presets.h = 15;
      presets.label = "PRESET";
      presets.styleSlot = i;
      presets.styleType = type;
      widgets_.push_back(presets);
    }

    Widget handle{};
    handle.kind = Kind::ModuleDragHandle;
    handle.param = static_cast<uint32_t>(i);
    handle.value = i;
    handle.x = x + 4; handle.y = y + 8; handle.w = colW - 8; handle.h = 52;
    handle.styleSlot = i;
    handle.styleType = type;
    widgets_.push_back(handle);

    Widget byp{};
    byp.kind = Kind::Pill;
    byp.param = slotParam(i, kSlotOffsetBypass);
    byp.x = x + 10; byp.y = y + 64; byp.w = colW - 20; byp.h = 22;
    byp.label = "BYPASS";
    byp.styleSlot = i;
    byp.styleType = type;
    widgets_.push_back(byp);

    const ModuleDesc* module = moduleDesc(type);
    const double cx1 = x + 38, cx2 = x + 105;
    if (module->controlCount > 0) {
      const int rows = (module->controlCount + 1) / 2;
      const double rowH = rows >= 5 ? 68.0 : (rows >= 4 ? 82.0 : 96.0);
      int toggleSlot = 0;
      for (int k = 0; k < module->controlCount; ++k) {
        const ControlDesc& control = module->controls[k];
        if (control.toggled) {
          // Mode buttons live at the bottom of the control area, just above
          // the MIX/GAIN labels, so they never sit on top of a knob.
          Widget t{};
          t.kind = Kind::ModuleToggle;
          t.param = slotParam(i, kSlotOffsetC0 + control.param);
          t.x = x + 10;
          t.y = y + 452.0 - toggleSlot * 30.0;
          t.w = colW - 20;
          t.h = 22;
          t.label = control.label;
          t.styleSlot = i;
          t.styleType = type;
          widgets_.push_back(t);
          ++toggleSlot;
          continue;
        }
        const double kx = (k & 1) ? cx2 : cx1;
        const double ky = y + 165.0 + (k / 2) * rowH;
        knob(slotParam(i, kSlotOffsetC0 + control.param), kx, ky, 18,
             control.label, i, true, control.param,
             control.stepped ? control.labels.count : 0);
      }
    }

    knob(slotParam(i, kSlotOffsetMix), x + colW * 0.30, y + 512, 13, "MIX", i);
    knob(slotParam(i, kSlotOffsetGain), x + colW * 0.70, y + 512, 13,
         "GAIN", i);

    // Full-card fallback drag handle. Controls are added first, so knobs and
    // buttons still win the hit test; clicking any empty part of the module
    // starts a drag or opens the browser.
    Widget card{};
    card.kind = Kind::ModuleDragHandle;
    card.param = static_cast<uint32_t>(i);
    card.value = i;
    card.x = x; card.y = y; card.w = colW; card.h = 556.0;
    card.styleSlot = i;
    card.styleType = type;
    widgets_.push_back(card);
  }

  if (state.browserSlot >= 0 && state.browserSlot < kSlotCount) {
    const double bx = 240, bw = 560;
    const int filterType = presetFilterTypeFor(shared, state);
    const double bh =
        browserHeight(state.browserCategory, state.browserQuery, filterType);
    const double by =
        browserY(state.browserCategory, state.browserQuery, filterType);
    for (int tab = 0; tab < 7; ++tab) {
      Widget t{};
      t.kind = Kind::BrowserTab;
      t.param = slotParam(state.browserSlot, kSlotOffsetType);
      t.value = tab;
      t.x = bx + 30 + tab * 74.0;
      t.y = by + 46;
      t.w = 70.0;
      t.h = 24.0;
      static const char* labels[7] = {"ALL", "EQ",     "COMP", "COLOUR",
                                      "DYN", "SPACE",  "PRESETS"};
      t.label = labels[tab];
      widgets_.push_back(t);
    }

    struct BEntry {
      int value = 0;
      int styleType = 0;
      const char* label = "";
      const char* subLabel = nullptr;
    };
    std::vector<BEntry> entries;
    if (state.browserCategory == 6) {
      const std::string needle = lowerText(state.browserQuery);
      for (int i = 0; i < kFactoryModulePresetCount; ++i) {
        const FactoryModulePreset& p = kFactoryModulePresets[i];
        if (filterType >= 0 && static_cast<int>(p.type) != filterType) continue;
        if (!needle.empty() &&
            lowerText(p.name).find(needle) == std::string::npos) {
          continue;
        }
        entries.push_back({-1000 - i, static_cast<int>(p.type), p.name,
                           p.source});
      }
      for (int i = 0; i < static_cast<int>(modulePresets_.size()); ++i) {
        const ModulePreset& p = modulePresets_[static_cast<size_t>(i)];
        if (filterType >= 0 && p.type != filterType) continue;
        if (!needle.empty() &&
            lowerText(p.name).find(needle) == std::string::npos) {
          continue;
        }
        entries.push_back({-2000 - i, p.type, p.name.c_str(),
                           "User module preset"});
      }
    } else {
      if (state.browserQuery.empty())
        entries.push_back({static_cast<int>(ModuleId::Off),
                           static_cast<int>(ModuleId::Off), "OFF", nullptr});
      for (int id = 1; id < kModuleCount; ++id) {
        if (!browserCategoryMatches(id, state.browserCategory)) continue;
        if (!browserModuleMatchesQuery(id, state.browserQuery)) continue;
        entries.push_back({id, id, slotModuleName(id), moduleSource(id)});
      }
    }
    const int columns = 4;
    const int visible = browserVisibleCount(state.browserQuery);
    const int maxScroll = std::max(0, static_cast<int>(entries.size()) - visible);
    const int scroll = std::clamp(state.browserScroll, 0, maxScroll);
    const double ow = 120.0, oh = 46.0, ox = 6.0, oy = 8.0;
    for (int k = scroll;
         k < static_cast<int>(entries.size()) && k < scroll + visible; ++k) {
      const int pos = k - scroll;
      const BEntry& entry = entries[static_cast<size_t>(k)];
      Widget b{};
      b.kind = Kind::BrowserOption;
      b.param = slotParam(state.browserSlot, kSlotOffsetType);
      b.value = entry.value;
      b.x = bx + 31 + static_cast<double>(pos % columns) * (ow + ox);
      b.y = by + 92 + static_cast<double>(pos / columns) * (oh + oy);
      b.w = ow;
      b.h = oh;
      b.label = entry.label;
      b.subLabel = entry.subLabel;
      b.styleType = entry.styleType;
      widgets_.push_back(b);
    }
    Widget search{};
    search.kind = Kind::BrowserSearch;
    search.param = slotParam(state.browserSlot, kSlotOffsetType);
    search.x = bx + 150;
    search.y = by + bh - 42;
    search.w = bw - 290;
    search.h = 28;
    search.label = "SEARCH";
    widgets_.push_back(search);

    if (maxScroll > 0) {
      Widget up{};
      up.kind = Kind::BrowserScroll;
      up.value = -1.0;
      up.x = bx + 18;
      up.y = by + bh - 42;
      up.w = 58;
      up.h = 28;
      up.label = "UP";
      widgets_.push_back(up);
      Widget down{};
      down.kind = Kind::BrowserScroll;
      down.value = 1.0;
      down.x = bx + 82;
      down.y = by + bh - 42;
      down.w = 58;
      down.h = 28;
      down.label = "DOWN";
      widgets_.push_back(down);
    }

    Widget cancel{};
    cancel.kind = Kind::BrowserOption;
    cancel.param = slotParam(state.browserSlot, kSlotOffsetType);
    cancel.value = -1;
    cancel.x = bx + bw - 130; cancel.y = by + bh - 42;
    cancel.w = 110; cancel.h = 28;
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
  text(cr, presetName(state.presetIndex), 237, 39, 13, 1, kText.r, kText.g,
       kText.b);

  // Wordmark stays centered; the compact A/B and COPY controls sit to its
  // right, before the PRESETS / SAVE / ABOUT strip.
  text(cr, "SUITE RACK", 520, 44, 21, 1, kText.r, kText.g, kText.b);

  const int ab = shared.abActiveSlot();
  roundRect(cr, 600, 20, 34, 28, 4);
  setRgb(cr, kAccent);
  cairo_fill(cr);
  text(cr, ab == 0 ? "A" : "B", 617, 39, 12, 1, 0.10, 0.08, 0.05);
  roundRect(cr, 640, 20, 56, 28, 4);
  setRgb(cr, Rgb{0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "COPY", 668, 39, 7.5, 1, kDim.r, kDim.g, kDim.b);

  struct Pill { const char* label; double x; };
  const Pill pills[] = {{"PRESETS", 706}, {"SAVE", 812}, {"ABOUT", 918}};
  for (const Pill& pill : pills) {
    const bool active = (std::string(pill.label) == "ABOUT" && state.aboutOpen) ||
                        (std::string(pill.label) == "SAVE" && state.savePreset) ||
                        (std::string(pill.label) == "PRESETS" && state.presetOpen);
    roundRect(cr, pill.x, 20, 96, 28, 4);
    setRgb(cr, active ? kAccent : Rgb{0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, pill.label, pill.x + 48, 39, 9, 1,
         active ? 0.10 : kDim.r, active ? 0.08 : kDim.g,
         active ? 0.05 : kDim.b);
  }
}

void Panel::drawRack(cairo_t* cr, const Shared& shared,
                     const PanelState& state) const {
  const double colW = 143.0, gap = 12.0, y = 92.0, h = 556.0;
  for (int i = 0; i < kSlotCount; ++i) {
    const double x = 60.0 + i * (colW + gap);
    const int type = static_cast<int>(shared.get(slotParam(i, kSlotOffsetType)) + 0.5);
    const ModuleStyle st = styleFor(type);
    roundRect(cr, x, y, colW, h, 6);
    setRgb(cr, st.body); cairo_fill(cr);
    const bool dropTarget = slotDragging_ && dropSlot_ == i;
    cairo_set_line_width(cr,
                         dropTarget ? 2.5 : (state.browserSlot == i ? 2.0 : 1.0));
    setRgb(cr, dropTarget ? st.accent : (state.browserSlot == i ? st.accent : st.edge),
           dropTarget ? 1.0 : (state.browserSlot == i ? 1.0 : 0.75));
    roundRect(cr, x + 0.5, y + 0.5, colW - 1, h - 1, 6); cairo_stroke(cr);
    // 500-series colour cap.
    roundRect(cr, x + 1, y + 1, colW - 2, 7, 4);
    setRgb(cr, st.accent, 0.95); cairo_fill(cr);
    char num[16];
    std::snprintf(num, sizeof(num), "SLOT %d", i + 1);
    text(cr, num, x + 10, y + 22, 8, 0, st.accent.r, st.accent.g, st.accent.b);
    if (moduleHasGr(type)) {
      drawGrMeter(cr, x + 10, y + 96, colW - 20, 8, state.slotGrDb[i]);
    }
    if (type == static_cast<int>(ModuleId::Off)) {
      text(cr, "EMPTY", x + colW * 0.5, y + 205, 15, 1, st.accent.r,
           st.accent.g, st.accent.b);
      text(cr, "CHOOSE A", x + colW * 0.5, y + 232, 7, 1, st.accent.r,
           st.accent.g, st.accent.b);
      text(cr, "MODULE", x + colW * 0.5, y + 245, 7, 1, st.accent.r,
           st.accent.g, st.accent.b);
    }
  }
  text(cr, "VIRTUAL 500 SERIES MIX RACK", 520, 88.5, 8.5, 1, kDim.r,
       kDim.g, kDim.b);
}

int Panel::browserOptionCount(int category, const std::string& query,
                              int presetFilterType) const {
  if (category == 6) {
    const std::string needle = lowerText(query);
    int count = 0;
    for (int i = 0; i < kFactoryModulePresetCount; ++i) {
      if (presetFilterType >= 0 &&
          static_cast<int>(kFactoryModulePresets[i].type) != presetFilterType) {
        continue;
      }
      if (!needle.empty() &&
          lowerText(kFactoryModulePresets[i].name).find(needle) ==
              std::string::npos) {
        continue;
      }
      ++count;
    }
    for (const auto& p : modulePresets_) {
      if (presetFilterType >= 0 && p.type != presetFilterType) continue;
      if (!needle.empty() &&
          lowerText(p.name).find(needle) == std::string::npos) {
        continue;
      }
      ++count;
    }
    return count;
  }
  return browserEntryCount(category, query);
}

double Panel::browserHeight(int category, const std::string& query,
                            int presetFilterType) const {
  const int count = browserOptionCount(category, query, presetFilterType);
  const int visible =
      query.empty() ? std::min(count, 16) : browserVisibleCount(query);
  const int rows = std::max(1, (visible + 3) / 4);
  return 92.0 + rows * 54.0 + 70.0;
}

double Panel::browserY(int category, const std::string& query,
                       int presetFilterType) const {
  const double h = browserHeight(category, query, presetFilterType);
  return std::max(84.0, (720.0 - h) * 0.5);
}

void Panel::drawBrowser(cairo_t* cr, const Shared& shared,
                        const PanelState& state) const {
  if (state.browserSlot < 0 || state.browserSlot >= kSlotCount) return;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.64);
  cairo_paint(cr);
  const double bx = 240, bw = 560;
  const double bh = browserHeight(state.browserCategory, state.browserQuery,
                                  presetFilterTypeFor(shared, state));
  const double by = browserY(state.browserCategory, state.browserQuery,
                             presetFilterTypeFor(shared, state));
  roundRect(cr, bx, by, bw, bh, 6);
  setRgb(cr, {0.13, 0.115, 0.10}); cairo_fill(cr);
  setRgb(cr, kAccent, 0.85); cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 6); cairo_stroke(cr);
  char title[48];
  std::snprintf(title, sizeof(title), "CHOOSE MODULE · SLOT %d",
                state.browserSlot + 1);
  text(cr, title, bx + bw * 0.5, by + 30, 14, 1, kText.r, kText.g, kText.b);
  const char* subtitle =
      state.browserCategory == 6
          ? (state.browserPresetFilter
                 ? "Presets for the loaded module · click one to load"
                 : "Factory and user module presets · click one to load")
          : "One job per module · 3–8 controls · no deep pages";
  text(cr, subtitle, bx + bw * 0.5, by + 86, 8.5, 1, kDim.r, kDim.g, kDim.b);
  if (state.browserCategory == 6 &&
      browserOptionCount(6, state.browserQuery,
                         presetFilterTypeFor(shared, state)) == 0) {
    text(cr, "No presets for this module yet", bx + bw * 0.5, by + 160, 10, 1,
         kDim.r, kDim.g, kDim.b);
  }
  const double tx = bx + 28, ty = by + 46;
  cairo_set_source_rgba(cr, 0.20, 0.17, 0.14, 0.7);
  cairo_rectangle(cr, tx, ty, 506, 24);
  cairo_fill(cr);

  // Search field, sharing the bottom row with UP/DOWN and CANCEL.
  const double sx = bx + 150, sy = by + bh - 42, sw = bw - 290, sh = 28;
  setRgb(cr, {0.07, 0.065, 0.06});
  roundRect(cr, sx, sy, sw, sh, 4);
  cairo_fill(cr);
  setRgb(cr, state.browserSearch ? kAccent : kEdge, 0.8);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, sx + 0.5, sy + 0.5, sw - 1, sh - 1, 4);
  cairo_stroke(cr);
  std::string shown =
      state.browserQuery.empty() ? "Search modules" : state.browserQuery;
  if (state.browserSearch && cursorVisible()) shown += "_";
  text(cr, shown.c_str(), sx + 10, sy + 18, 9.0, 0,
       state.browserQuery.empty() ? kDim.r : kText.r,
       state.browserQuery.empty() ? kDim.g : kText.g,
       state.browserQuery.empty() ? kDim.b : kText.b);
}

int Panel::aboutMaxScroll() const {
  const int total = kModuleCount - 1;
  const int visible = 26;  // 13 rows x 2 columns
  if (total <= visible) return 0;
  return (total - visible + 1) / 2;
}

void Panel::drawBrowserMenu(cairo_t* cr, const PanelState& state) const {
  if (state.modulePresetMenu < 0 ||
      state.modulePresetMenu >= static_cast<int>(modulePresets_.size())) {
    return;
  }
  const ModulePreset& preset =
      modulePresets_[static_cast<size_t>(state.modulePresetMenu)];
  const double x = 340.0, y = 252.0, w = 300.0, h = 150.0;
  setRgb(cr, {0.0, 0.0, 0.0}, 0.45);
  cairo_paint(cr);
  setRgb(cr, {0.02, 0.02, 0.03}, 0.55);
  roundRect(cr, x + 4, y + 5, w, h, 6);
  cairo_fill(cr);
  setRgb(cr, {0.12, 0.105, 0.09});
  roundRect(cr, x, y, w, h, 6);
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.9);
  cairo_set_line_width(cr, 1.3);
  roundRect(cr, x + 0.5, y + 0.5, w - 1, h - 1, 6);
  cairo_stroke(cr);

  text(cr, "MODULE PRESET", x + w * 0.5, y + 24, 11, 1, kText.r, kText.g,
       kText.b);
  text(cr, preset.name.c_str(), x + w * 0.5, y + 50, 10, 1, kAccent.r,
       kAccent.g, kAccent.b);

  const double by = y + 84;
  roundRect(cr, x + 16, by, 80, 26, 4);
  setRgb(cr, {0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "RENAME", x + 56, by + 17, 8, 1, kText.r, kText.g, kText.b);
  roundRect(cr, x + 104, by, 92, 26, 4);
  setRgb(cr, state.deleteModuleConfirm ? kAccent : Rgb{0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, state.deleteModuleConfirm ? "CONFIRM" : "DELETE", x + 150, by + 17,
       8, 1, state.deleteModuleConfirm ? 0.10 : kText.r,
       state.deleteModuleConfirm ? 0.08 : kText.g,
       state.deleteModuleConfirm ? 0.05 : kText.b);
  roundRect(cr, x + 204, by, 80, 26, 4);
  setRgb(cr, {0.12, 0.11, 0.10});
  cairo_fill(cr);
  text(cr, "CANCEL", x + 244, by + 17, 8, 1, kText.r, kText.g, kText.b);
}

void Panel::drawPresetList(cairo_t* cr, const Shared& /*shared*/,
                           const PanelState& state) const {
  if (!state.presetOpen) return;
  const int count = presetCount();
  const int rows = presetRowsFor(count);
  const int maxScroll = std::max(0, count - rows);
  const int scroll = std::clamp(state.presetScroll, 0, maxScroll);
  const double h = kPresetHeaderH + rows * kPresetRowH +
                   kPresetBottomGap + kPresetFooterH + 30.0;

  setRgb(cr, {0.02, 0.02, 0.03}, 0.55);
  roundRect(cr, kPresetListX + 4, kPresetListY + 5, kPresetListW, h, 6);
  cairo_fill(cr);
  setRgb(cr, {0.12, 0.105, 0.09});
  roundRect(cr, kPresetListX, kPresetListY, kPresetListW, h, 6);
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.9);
  cairo_set_line_width(cr, 1.3);
  roundRect(cr, kPresetListX + 0.5, kPresetListY + 0.5, kPresetListW - 1,
            h - 1, 6);
  cairo_stroke(cr);

  text(cr, "RACK PRESETS", kPresetListX + 14, kPresetListY + 20, 12, 0,
       kText.r, kText.g, kText.b);
  text(cr, "Factory racks and your saved racks", kPresetListX + 14,
       kPresetListY + 34, 7.0, 0, kDim.r, kDim.g, kDim.b);

  for (int row = 0; row < rows; ++row) {
    const int index = scroll + row;
    if (index >= count) break;
    const double y = kPresetListY + kPresetHeaderH + row * kPresetRowH;
    const bool current = index == state.presetIndex;
    if (current) {
      setRgb(cr, kAccent, 0.22);
      cairo_rectangle(cr, kPresetListX + 6, y, kPresetListW - 12,
                      kPresetRowH - 1);
      cairo_fill(cr);
    }
    text(cr, presetName(index), kPresetListX + 14, y + 13, 8.5, 0,
         current ? kAccent.r : kText.r, current ? kAccent.g : kText.g,
         current ? kAccent.b : kText.b);
    text(cr, index < kPresetCount ? kPresets[index].category : "User",
         kPresetListX + kPresetListW - 14, y + 13, 6.5, 2,
         current ? kAccent.r : kDim.r, current ? kAccent.g : kDim.g,
         current ? kAccent.b : kDim.b);
    if (row < rows - 1) {
      setRgb(cr, kEdge, 0.18);
      cairo_set_line_width(cr, 0.6);
      cairo_move_to(cr, kPresetListX + 14, y + kPresetRowH - 1);
      cairo_line_to(cr, kPresetListX + kPresetListW - 14,
                    y + kPresetRowH - 1);
      cairo_stroke(cr);
    }
  }

  const double footerY =
      kPresetListY + kPresetHeaderH + rows * kPresetRowH + kPresetBottomGap + 4;
  if (maxScroll > 0) {
    roundRect(cr, kPresetListX + 14, footerY, 54, 24, 4);
    setRgb(cr, scroll > 0 ? Rgb{0.20, 0.17, 0.14} : Rgb{0.12, 0.11, 0.10});
    cairo_fill(cr);
    text(cr, "UP", kPresetListX + 41, footerY + 16, 8, 1, kText.r, kText.g,
         kText.b);
    roundRect(cr, kPresetListX + 74, footerY, 54, 24, 4);
    setRgb(cr, scroll < maxScroll ? Rgb{0.20, 0.17, 0.14}
                                  : Rgb{0.12, 0.11, 0.10});
    cairo_fill(cr);
    text(cr, "DOWN", kPresetListX + 101, footerY + 16, 8, 1, kText.r,
         kText.g, kText.b);
  }
  const bool userCurrent = state.presetIndex >= kPresetCount &&
                           state.presetIndex < count;
  if (userCurrent) {
    roundRect(cr, kPresetListX + 140, footerY, 68, 24, 4);
    setRgb(cr, {0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, "RENAME", kPresetListX + 174, footerY + 16, 7.5, 1, kText.r,
         kText.g, kText.b);
    roundRect(cr, kPresetListX + 214, footerY, 72, 24, 4);
    setRgb(cr, state.deletePresetConfirm ? kAccent : Rgb{0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, state.deletePresetConfirm ? "CONFIRM" : "DELETE",
         kPresetListX + 250, footerY + 16, 7.5, 1,
         state.deletePresetConfirm ? 0.10 : kText.r,
         state.deletePresetConfirm ? 0.08 : kText.g,
         state.deletePresetConfirm ? 0.05 : kText.b);
  }
  roundRect(cr, kPresetListX + kPresetListW - 84, footerY, 70, 24, 4);
  setRgb(cr, {0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "CLOSE", kPresetListX + kPresetListW - 49, footerY + 16, 8, 1,
       kText.r, kText.g, kText.b);

  const double row2 = footerY + 30;
  if (state.presetIndex >= 0 && state.presetIndex < count) {
    roundRect(cr, kPresetListX + 14, row2, 80, 24, 4);
    setRgb(cr, {0.20, 0.17, 0.14});
    cairo_fill(cr);
    text(cr, "EXPORT", kPresetListX + 54, row2 + 16, 7.5, 1, kText.r, kText.g,
         kText.b);
  }
  roundRect(cr, kPresetListX + 104, row2, 80, 24, 4);
  setRgb(cr, {0.20, 0.17, 0.14});
  cairo_fill(cr);
  text(cr, "IMPORT", kPresetListX + 144, row2 + 16, 7.5, 1, kText.r, kText.g,
       kText.b);
}

void Panel::drawAbout(cairo_t* cr, const Shared& /*shared*/,
                      const PanelState& state) const {
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.72);
  cairo_paint(cr);
  const double bx = 100, by = 50, bw = 840, bh = 650;
  roundRect(cr, bx, by, bw, bh, 7);
  setRgb(cr, {0.12, 0.105, 0.09});
  cairo_fill(cr);
  setRgb(cr, kAccent, 0.9);
  cairo_set_line_width(cr, 1.5);
  roundRect(cr, bx + 0.5, by + 0.5, bw - 1, bh - 1, 7);
  cairo_stroke(cr);

  text(cr, "ABOUT SUITE RACK", 520, 92, 20, 1, kText.r, kText.g, kText.b);
  text(cr, "Virtual 500 Series Mix Rack · modules, sources and character",
       520, 114, 8, 1, kDim.r, kDim.g, kDim.b);

  const double colX[2] = {120.0, 530.0};
  const double headerY = 144.0;
  for (int col = 0; col < 2; ++col) {
    text(cr, "MODULE", colX[col], headerY, 7, 0, kDim.r, kDim.g, kDim.b);
    text(cr, "SOURCE", colX[col] + 104, headerY, 7, 0, kDim.r, kDim.g, kDim.b);
    text(cr, "BASED ON / CHARACTER", colX[col] + 218, headerY, 7, 0, kDim.r,
         kDim.g, kDim.b);
  }

  const int rows = 13;
  const int start = state.aboutScroll * 2;
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < 2; ++col) {
      const int index = start + row * 2 + col;
      const int id = index + 1;
      if (id >= kModuleCount) continue;
      const double x = colX[col];
      const double y = 162.0 + row * 26.0;
      const ModuleDesc* d = moduleDesc(id);
      const ModuleStyle st = styleFor(id);
      text(cr, d->name, x, y, 8.5, 0, st.accent.r, st.accent.g, st.accent.b);
      text(cr, moduleSource(id), x + 104, y, 6.8, 0, kDim.r, kDim.g, kDim.b);
      text(cr, d->description, x + 218, y, 6.5, 0, kText.r, kText.g,
           kText.b);
      setRgb(cr, kEdge, 0.20);
      cairo_set_line_width(cr, 0.6);
      cairo_move_to(cr, x, y + 8);
      cairo_line_to(cr, x + 380, y + 8);
      cairo_stroke(cr);
    }
  }

  const int maxScroll = aboutMaxScroll();
  if (maxScroll > 0) {
    roundRect(cr, 120, 604, 58, 26, 4);
    setRgb(cr, state.aboutScroll > 0 ? Rgb{0.20, 0.17, 0.14}
                                     : Rgb{0.12, 0.11, 0.10});
    cairo_fill(cr);
    text(cr, "UP", 149, 621, 8, 1, kText.r, kText.g, kText.b);
    roundRect(cr, 184, 604, 58, 26, 4);
    setRgb(cr, state.aboutScroll < maxScroll ? Rgb{0.20, 0.17, 0.14}
                                             : Rgb{0.12, 0.11, 0.10});
    cairo_fill(cr);
    text(cr, "DOWN", 213, 621, 8, 1, kText.r, kText.g, kText.b);
    char pos[64];
    const int first = start + 1;
    const int last = std::min(start + rows * 2, kModuleCount - 1);
    std::snprintf(pos, sizeof(pos), "Modules %d-%d of %d  ·  scroll wheel",
                  first, last, kModuleCount - 1);
    text(cr, pos, 262, 621, 7, 0, kDim.r, kDim.g, kDim.b);
  }

  text(cr, "Click a module name or empty card area to open its browser. Drag it to reorder slots.",
       120, 520, 7.5, 0, kText.r, kText.g, kText.b);
  text(cr, "Double-click a knob to reset it. Right-click a knob to type a value.",
       120, 534, 7.5, 0, kText.r, kText.g, kText.b);
  text(cr, "Suite Rack is GPL-3.0-or-later. Shared module DSP lives in hearth-dsp.",
       120, 554, 7.0, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "JClones-derived ports use MIT-licensed JClones DSP; see THIRD_PARTY_NOTICES.md.",
       120, 568, 7.0, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "Jost font: SIL OFL 1.1. CLAP headers: MIT. Exclusive modules are unique to this plugin.",
       120, 582, 7.0, 0, kDim.r, kDim.g, kDim.b);

  roundRect(cr, 820, 654, 80, 26, 4);
  setRgb(cr, kAccent);
  cairo_fill(cr);
  text(cr, "CLOSE", 860, 672, 9, 1, 0.10, 0.08, 0.05);
}

void Panel::draw(cairo_t* cr, Shared& shared, const PanelState& state) {
  layout(shared, state);
  cairo_set_source_rgb(cr, 0.07, 0.065, 0.06);
  cairo_paint(cr);
  drawHeader(cr, shared, state);
  drawRack(cr, shared, state);
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::BrowserOption && w.kind != Kind::BrowserTab &&
        w.kind != Kind::BrowserScroll) {
      drawWidget(cr, w, shared, state);
    }
  }
  drawBrowser(cr, shared, state);
  for (const Widget& w : widgets_) {
    if (w.kind == Kind::BrowserOption || w.kind == Kind::BrowserTab ||
        w.kind == Kind::BrowserScroll) {
      drawWidget(cr, w, shared, state);
    }
  }
  if (state.modulePresetMenu >= 0) drawBrowserMenu(cr, state);
  if (slotDragging_) drawDragGhost(cr, shared);
  drawLevelMeter(cr, 20, 100, 14, 520, state.inLevelL, state.inLevelR);
  drawLevelMeter(cr, 1006, 100, 14, 520, state.outLevelL, state.outLevelR);
  text(cr, "IN", 27, 640, 8, 1, kDim.r, kDim.g, kDim.b);
  text(cr, "OUT", 1013, 640, 8, 1, kDim.r, kDim.g, kDim.b);
  text(cr, "INPUT", 96, 681, 7.5, 2, kDim.r, kDim.g, kDim.b);
  text(cr, "OUTPUT", 904, 681, 7.5, 0, kDim.r, kDim.g, kDim.b);
  text(cr, "SUITE RACK · VIRTUAL 500 SERIES MIX RACK · SIX SLOTS", 520, 714, 10.5, 1,
       kText.r, kText.g, kText.b);
  if (state.presetOpen) drawPresetList(cr, shared, state);
  if (state.aboutOpen) drawAbout(cr, shared, state);
  drawTextEntry(cr, state);
}

void Panel::drawWidget(cairo_t* cr, const Widget& w, const Shared& shared,
                       const PanelState& state) const {
  const int type = w.styleSlot >= 0
                       ? static_cast<int>(shared.get(slotParam(w.styleSlot, kSlotOffsetType)) + 0.5)
                       : -1;
  const ModuleStyle st = styleFor(type);
  switch (w.kind) {
    case Kind::Power: {
      // Shared Comp Suite power button, unchanged.
      const bool on = shared.get(P_POWER) > 0.5;
      cairo_arc(cr, w.x, w.y, w.r, 0, 2 * kPi);
      setRgb(cr, {0.12, 0.12, 0.12});
      cairo_fill(cr);
      cairo_arc(cr, w.x, w.y, w.r - 1.5, 0, 2 * kPi);
      setRgb(cr, on ? Rgb{0.36, 0.86, 0.46} : Rgb{0.42, 0.42, 0.42});
      cairo_set_line_width(cr, 2.2);
      cairo_new_path(cr);
      cairo_arc(cr, w.x, w.y, 7, -kPi / 2 + 0.6, -kPi / 2 + 2 * kPi - 0.6);
      cairo_stroke(cr);
      cairo_move_to(cr, w.x, w.y - 9);
      cairo_line_to(cr, w.x, w.y - 1);
      cairo_stroke(cr);
      break;
    }
    case Kind::SlotCard:
    case Kind::ModuleDragHandle:
      break;
    case Kind::ModuleName: {
      const int slot = static_cast<int>(w.value);
      const int type = static_cast<int>(
          shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, st.body); cairo_fill(cr);
      setRgb(cr, st.edge, 0.9); cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4); cairo_stroke(cr);
      text(cr, slotModuleName(type), w.x + 12, w.y + w.h * 0.5 + 5, 12, 0,
           st.accent.r, st.accent.g, st.accent.b);
      if (type == static_cast<int>(ModuleId::Off)) {
        text(cr, "CHOOSE", w.x + w.w - 10, w.y + w.h * 0.5 + 4, 7, 2,
             kDim.r, kDim.g, kDim.b);
      }
      break;
    }
    case Kind::BrowserTab: {
      const bool active = static_cast<int>(w.value) == state.browserCategory;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, active ? kAccent : Rgb{0.20, 0.17, 0.14});
      cairo_fill(cr);
      setRgb(cr, kEdge, 0.8);
      cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 9, 1,
           active ? 0.10 : kText.r, active ? 0.08 : kText.g,
           active ? 0.05 : kText.b);
      break;
    }
    case Kind::BrowserScroll: {
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, {0.20, 0.17, 0.14});
      cairo_fill(cr);
      setRgb(cr, kEdge, 0.8);
      cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 9, 1,
           kText.r, kText.g, kText.b);
      break;
    }
    case Kind::ModulePresetButton: {
      roundRect(cr, w.x, w.y, w.w, w.h, 3);
      setRgb(cr, st.body);
      cairo_fill(cr);
      setRgb(cr, st.accent, 0.85);
      cairo_set_line_width(cr, 0.9);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 3);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + 10, 6.0, 1, st.accent.r,
           st.accent.g, st.accent.b);
      break;
    }
    case Kind::BrowserSearch:
      break;
    case Kind::BrowserOption: {
      const bool cancel = w.value == -1;
      const bool current = !cancel && w.value >= 0 &&
          std::fabs(shared.get(w.param) - w.value) < 0.5;
      const ModuleStyle bst = cancel ? styleFor(0) : styleFor(w.styleType);
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, current ? bst.accent : bst.body);
      cairo_fill(cr);
      setRgb(cr, bst.edge, 0.9); cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4); cairo_stroke(cr);
      auto textWidth = [&](const std::string& text, double size) {
        cairo_set_font_face(cr, font_);
        cairo_set_font_size(cr, size);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, text.c_str(), &extents);
        return extents.x_advance;
      };
      const std::string label = w.label ? w.label : "";
      const double avail = w.w - 10.0;
      double nameSize = cancel ? 9.0 : 13.0;
      std::string line1 = label;
      std::string line2;
      if (!cancel && textWidth(label, 13.0) > avail) {
        if (textWidth(label, 11.0) <= avail) {
          nameSize = 11.0;
        } else {
          nameSize = 10.2;
          std::vector<std::string> words;
          size_t start = 0;
          while (start < label.size()) {
            const size_t space = label.find(' ', start);
            words.push_back(label.substr(
                start, space == std::string::npos ? std::string::npos
                                                  : space - start));
            if (space == std::string::npos) break;
            start = space + 1;
          }
          for (double size = 10.2; size >= 8.2; size -= 0.4) {
            line1.clear();
            line2.clear();
            for (const auto& word : words) {
              const std::string candidate =
                  line1.empty() ? word : line1 + " " + word;
              if (line1.empty() || textWidth(candidate, size) <= avail) {
                line1 = candidate;
              } else if (line2.empty()) {
                line2 = word;
              } else {
                const std::string extended = line2 + " " + word;
                if (textWidth(extended, size) <= avail) line2 = extended;
              }
            }
            if (line2.empty() || textWidth(line2, size) <= avail) {
              nameSize = size;
              break;
            }
          }
        }
      }
      const double nameR = current ? 0.08 : bst.accent.r;
      const double nameG = current ? 0.06 : bst.accent.g;
      const double nameB = current ? 0.04 : bst.accent.b;
      if (line2.empty()) {
        text(cr, line1.c_str(), w.x + w.w * 0.5, w.y + w.h * 0.5 + 2,
             nameSize, 1, nameR, nameG, nameB);
      } else {
        text(cr, line1.c_str(), w.x + w.w * 0.5, w.y + 17, nameSize, 1,
             nameR, nameG, nameB);
        text(cr, line2.c_str(), w.x + w.w * 0.5, w.y + 28, nameSize, 1,
             nameR, nameG, nameB);
      }
      if (!cancel) {
        const char* src = w.subLabel;
        if (src && *src) {
          text(cr, src, w.x + w.w * 0.5,
               w.y + (line2.empty() ? w.h - 7 : w.h - 4),
               line2.empty() ? 6.0 : 5.2, 1,
               current ? 0.10 : kDim.r, current ? 0.08 : kDim.g,
               current ? 0.06 : kDim.b);
        }
      }
      break;
    }
    case Kind::ModuleToggle: {
      const bool on = shared.get(w.param) > 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, on ? st.accent : st.body);
      cairo_fill(cr);
      setRgb(cr, st.edge, 0.9);
      cairo_set_line_width(cr, 1.0);
      roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
      cairo_stroke(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 8, 1,
           on ? 0.08 : st.accent.r, on ? 0.06 : st.accent.g,
           on ? 0.04 : st.accent.b);
      break;
    }
    case Kind::ModuleButton: {
      const bool lit = std::fabs(shared.get(w.param) - w.value) < 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, lit ? st.accent : st.body); cairo_fill(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
           lit ? 0.08 : st.accent.r, lit ? 0.06 : st.accent.g,
           lit ? 0.04 : st.accent.b);
      break;
    }
    case Kind::ModelButton: {
      const bool lit = std::fabs(shared.get(w.param) - w.value) < 0.5;
      roundRect(cr, w.x, w.y, w.w, w.h, 4);
      setRgb(cr, lit ? st.accent : st.body); cairo_fill(cr);
      text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10, 1,
           lit ? 0.08 : st.accent.r, lit ? 0.06 : st.accent.g,
           lit ? 0.04 : st.accent.b);
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
      drawKnob(cr, w, shared, shared.get(w.param));
      break;
  }
}

void Panel::drawKnob(cairo_t* cr, const Widget& w, const Shared& shared,
                     double value) const {
  const ParamSpec& s = kParamSpecs[w.param];
  const bool stepped01 = w.steps > 1 && w.moduleControl >= 0;
  const double vmax = (w.steps > 1 && !stepped01) ? w.steps - 1 : s.max;
  const double span = vmax - s.min;
  const double f = clamp01(span > 0.0 ? (value - s.min) / span : 0.0);
  const double a = knobAngle(f);
  const int knobType = w.styleSlot >= 0 ? w.styleType : -1;
  const ModuleStyle st = styleFor(knobType);
  setRgb(cr, st.accent, 0.45);
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
  setRgb(cr, st.body); cairo_fill(cr);
  cairo_arc(cr, w.x, w.y, w.r * 0.82, 0, 2 * kPi);
  setRgb(cr, st.knob); cairo_fill(cr);
  cairo_set_line_width(cr, 2.0);
  setRgb(cr, st.accent);
  cairo_arc(cr, w.x, w.y, w.r + 2, knobAngle(0), a); cairo_stroke(cr);
  cairo_set_line_width(cr, 2.4);
  setRgb(cr, st.body);
  cairo_move_to(cr, w.x + std::cos(a) * w.r * 0.2, w.y + std::sin(a) * w.r * 0.2);
  cairo_line_to(cr, w.x + std::cos(a) * w.r * 0.65, w.y + std::sin(a) * w.r * 0.65);
  cairo_stroke(cr);
  const double labelY = w.labelBelow ? w.y + w.r * 1.5 + 20
                                     : w.y - w.r * 1.5 - (w.compact ? 9.0 : 16.0);
  text(cr, w.label, w.x, labelY, w.compact ? 8.5 : 10.0, 1,
       kText.r, kText.g, kText.b);
  const std::string val = formatParamValue(shared, w.param, value);
  text(cr, val.c_str(), w.x, w.y + w.r + (w.compact ? 11.0 : 15.0),
       w.compact ? 8.5 : 8.5, 1, st.accent.r, st.accent.g, st.accent.b);
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

void Panel::drawGrMeter(cairo_t* cr, double x, double y, double w, double h,
                       double grDb) const {
  roundRect(cr, x, y, w, h, 2.5);
  setRgb(cr, {0.055, 0.052, 0.048});
  cairo_fill(cr);
  const int segments = 12;
  const double gap = 1.0;
  const double segW = (w - 2.0 - gap * (segments - 1)) / segments;
  const double db = std::clamp(grDb, 0.0, 12.0);
  for (int i = 0; i < segments; ++i) {
    const double sx = x + 1.0 + i * (segW + gap);
    const bool on = db >= static_cast<double>(i + 1);
    Rgb c = {0.16, 0.14, 0.12};
    if (on) {
      if (i < 3) c = {0.36, 0.80, 0.42};
      else if (i < 6) c = {0.94, 0.62, 0.20};
      else c = {0.95, 0.26, 0.18};
    }
    setRgb(cr, c, on ? 1.0 : 0.55);
    cairo_rectangle(cr, sx, y + 1.5, segW, h - 3.0);
    cairo_fill(cr);
  }
  text(cr, "GR", x, y - 3.0, 7.0, 0, kDim.r, kDim.g, kDim.b);
  const double ticks[5] = {0.0, 3.0, 6.0, 9.0, 12.0};
  for (double t : ticks) {
    const double tx = x + 1.0 + (w - 2.0) * (t / 12.0);
    char label[8];
    std::snprintf(label, sizeof(label), "%.0f", t);
    text(cr, label, tx, y + h + 6.0, 7.0, 1, kDim.r, kDim.g, kDim.b);
  }
}

bool Panel::hitWidget(const Widget& w, double x, double y) const {
  if (w.kind == Kind::Knob) {
    const double dx = x - w.x, dy = y - w.y;
    return dx * dx + dy * dy <= w.r * w.r * 1.5;
  }
  if (w.kind == Kind::Power) {
    const double dx = x - w.x, dy = y - w.y;
    return dx * dx + dy * dy <= w.r * w.r * 1.3;
  }
  return x >= w.x && x <= w.x + w.w && y >= w.y && y <= w.y + w.h;
}

int Panel::slotAtX(double x) const {
  constexpr double colW = 143.0;
  constexpr double gap = 12.0;
  constexpr double start = 60.0;
  const double center = start + colW * 0.5;
  const int slot =
      static_cast<int>(std::lround((x - center) / (colW + gap)));
  return std::clamp(slot, 0, kSlotCount - 1);
}

void Panel::moveSlot(Shared& shared, int from, int to) {
  if (from == to || from < 0 || to < 0 || from >= kSlotCount ||
      to >= kSlotCount) {
    return;
  }
  double temp[kSlotStride];
  for (uint32_t off = 0; off < kSlotStride; ++off) {
    temp[off] = shared.get(slotParam(from, static_cast<int>(off)));
  }
  if (from < to) {
    for (int slot = from; slot < to; ++slot) {
      for (uint32_t off = 0; off < kSlotStride; ++off) {
        shared.set(slotParam(slot, static_cast<int>(off)),
                   shared.get(slotParam(slot + 1, static_cast<int>(off))));
      }
    }
  } else {
    for (int slot = from; slot > to; --slot) {
      for (uint32_t off = 0; off < kSlotStride; ++off) {
        shared.set(slotParam(slot, static_cast<int>(off)),
                   shared.get(slotParam(slot - 1, static_cast<int>(off))));
      }
    }
  }
  for (uint32_t off = 0; off < kSlotStride; ++off) {
    shared.set(slotParam(to, static_cast<int>(off)), temp[off]);
  }
}

void Panel::drawDragGhost(cairo_t* cr, const Shared& shared) const {
  if (dragSlot_ < 0) return;
  const int type =
      static_cast<int>(shared.get(slotParam(dragSlot_, kSlotOffsetType)) + 0.5);
  const ModuleStyle st = styleFor(type);
  const double x = dragX_ + 12.0;
  const double y = dragY_ - 20.0;
  const double w = 132.0;
  const double h = 34.0;
  roundRect(cr, x + 2, y + 2, w, h, 5);
  setRgb(cr, {0.0, 0.0, 0.0}, 0.35);
  cairo_fill(cr);
  roundRect(cr, x, y, w, h, 5);
  setRgb(cr, st.body, 0.98);
  cairo_fill(cr);
  setRgb(cr, st.accent, 0.95);
  cairo_set_line_width(cr, 1.5);
  roundRect(cr, x + 0.5, y + 0.5, w - 1, h - 1, 5);
  cairo_stroke(cr);
  text(cr, slotModuleName(type), x + w * 0.5, y + h * 0.5 + 4, 12, 1,
       st.accent.r, st.accent.g, st.accent.b);
}

void Panel::beginSlotDrag(PanelState& state, int slot, double x, double y) {
  state.selectedSlot = slot;
  dragSlot_ = slot;
  dropSlot_ = slot;
  slotDragging_ = false;
  dragStartX_ = x;
  dragStartY_ = y;
  dragX_ = x;
  dragY_ = y;
}

bool Panel::pointerDown(Shared& shared, PanelState& state, double x, double y,
                        bool fine) {
  (void)fine;
  if (state.saveModule) {
    state.saveModule = false;
    state.saveModuleSlot = -1;
    state.textBuffer.clear();
  } else if (state.savePreset) {
    state.savePreset = false;
    state.textBuffer.clear();
  } else if (state.textParam >= 0) {
    textKey(shared, state, kTextEnter, 0);
  }
  if (state.aboutOpen) {
    if (x >= 120 && x <= 178 && y >= 604 && y <= 630) {
      state.aboutScroll = std::max(0, state.aboutScroll - 1);
      return true;
    }
    if (x >= 184 && x <= 242 && y >= 604 && y <= 630) {
      state.aboutScroll = std::min(aboutMaxScroll(), state.aboutScroll + 1);
      return true;
    }
    if ((x >= 820 && x <= 900 && y >= 654 && y <= 680) ||
        x < 100 || x > 940 || y < 50 || y > 700) {
      state.aboutOpen = false;
    }
    return true;
  }
  if (x >= 600 && x <= 634 && y >= 20 && y <= 48) {
    shared.abSwitch(1 - shared.abActiveSlot());
    state.presetDirty = true;
    return true;
  }
  if (x >= 640 && x <= 696 && y >= 20 && y <= 48) {
    const int active = shared.abActiveSlot();
    shared.abCopy(active, 1 - active);
    state.presetDirty = true;
    return true;
  }
  if (x >= 918 && x <= 1014 && y >= 20 && y <= 48) {
    state.aboutOpen = true;
    state.aboutScroll = 0;
    state.presetOpen = 0;
    return true;
  }
  if (x >= 706 && x <= 802 && y >= 20 && y <= 48) {
    state.presetOpen = state.presetOpen ? 0 : 1;
    state.browserSlot = -1;
    state.deletePresetConfirm = false;
    if (state.presetOpen) {
      const int rows = presetRowsFor(presetCount());
      const int maxScroll = std::max(0, presetCount() - rows);
      state.presetScroll = state.presetIndex >= 0
                               ? std::clamp(state.presetIndex - rows + 1, 0,
                                            maxScroll)
                               : 0;
    }
    return true;
  }
  if (state.presetOpen) {
    const int count = presetCount();
    const int rows = presetRowsFor(count);
    const int maxScroll = std::max(0, count - rows);
    const int scroll = std::clamp(state.presetScroll, 0, maxScroll);
    const double footerY =
      kPresetListY + kPresetHeaderH + rows * kPresetRowH + kPresetBottomGap + 4;
    if (x >= kPresetListX + 14 && x <= kPresetListX + 68 &&
        y >= footerY && y <= footerY + 24) {
      state.presetScroll = std::max(0, scroll - 1);
      return true;
    }
    if (x >= kPresetListX + 74 && x <= kPresetListX + 128 &&
        y >= footerY && y <= footerY + 24) {
      state.presetScroll = std::min(maxScroll, scroll + 1);
      return true;
    }
    const bool userCurrent = state.presetIndex >= kPresetCount &&
                             state.presetIndex < count;
    if (userCurrent && x >= kPresetListX + 140 && x <= kPresetListX + 208 &&
        y >= footerY && y <= footerY + 24) {
      state.renamePreset = true;
      state.textBuffer = presetName(state.presetIndex);
      state.textFresh = true;
      state.deletePresetConfirm = false;
      state.presetOpen = 0;
      return true;
    }
    if (userCurrent && x >= kPresetListX + 214 && x <= kPresetListX + 286 &&
        y >= footerY && y <= footerY + 24) {
      if (!state.deletePresetConfirm) {
        state.deletePresetConfirm = true;
        return true;
      }
      const std::string name = presetName(state.presetIndex);
      if (deleteUserPreset(userPresetDir(), name)) {
        userPresets_ = loadUserPresets(userPresetDir());
        state.presetIndex = -1;
        state.presetDirty = true;
      }
      state.deletePresetConfirm = false;
      state.presetOpen = 0;
      return true;
    }
    state.deletePresetConfirm = false;
    const double row2 = footerY + 30;
    if (state.presetIndex >= 0 && state.presetIndex < count &&
        x >= kPresetListX + 14 && x <= kPresetListX + 94 &&
        y >= row2 && y <= row2 + 24) {
      exportPreset(shared, state);
      return true;
    }
    if (x >= kPresetListX + 104 && x <= kPresetListX + 184 &&
        y >= row2 && y <= row2 + 24) {
      importPresetDialog(shared, state);
      return true;
    }
    if (x >= kPresetListX + kPresetListW - 84 &&
        x <= kPresetListX + kPresetListW - 14 && y >= footerY &&
        y <= footerY + 24) {
      state.presetOpen = 0;
      return true;
    }
    if (x >= kPresetListX && x <= kPresetListX + kPresetListW &&
        y >= kPresetListY + kPresetHeaderH &&
        y < kPresetListY + kPresetHeaderH + rows * kPresetRowH) {
      const int row = static_cast<int>((y - kPresetListY - kPresetHeaderH) /
                                       kPresetRowH);
      const int index = scroll + row;
      if (index >= 0 && index < count) {
        applyPreset(shared, state, index);
      }
      state.presetOpen = 0;
      return true;
    }
    if (y >= kPresetListY) {
      state.presetOpen = 0;
      state.deletePresetConfirm = false;
      return true;
    }
  }
  if (x >= 812 && x <= 908 && y >= 20 && y <= 48) {
    state.savePreset = true;
    state.textFresh = true;
    state.textParam = -1;
    if (state.presetIndex >= kPresetCount) {
      state.textBuffer = presetName(state.presetIndex);
    } else {
      state.textBuffer = "My Rack";
    }
    return true;
  }
  if (handlePresetClick(shared, state, x, y)) return true;
  if (state.browserSlot >= 0) {
    if (state.modulePresetMenu >= 0) {
      const double mx = 340.0, my = 252.0;
      const double by = my + 84.0;
      if (x >= mx + 16 && x <= mx + 96 && y >= by && y <= by + 26) {
        const int idx = state.modulePresetMenu;
        if (idx >= 0 && idx < static_cast<int>(modulePresets_.size())) {
          state.renameModulePreset = true;
          state.renameModuleIndex = idx;
          state.textBuffer = modulePresets_[static_cast<size_t>(idx)].name;
          state.textFresh = true;
          state.modulePresetMenu = -1;
          state.deleteModuleConfirm = false;
          state.browserSlot = -1;
          state.browserQuery.clear();
          state.browserSearch = false;
          state.browserPresetFilter = false;
        }
        return true;
      }
      if (x >= mx + 104 && x <= mx + 196 && y >= by && y <= by + 26) {
        if (!state.deleteModuleConfirm) {
          state.deleteModuleConfirm = true;
          return true;
        }
        const int idx = state.modulePresetMenu;
        if (idx >= 0 && idx < static_cast<int>(modulePresets_.size())) {
          const std::string name =
              modulePresets_[static_cast<size_t>(idx)].name;
          if (deleteModulePreset(modulePresetDir(), name)) {
            modulePresets_ = loadModulePresets(modulePresetDir());
          }
        }
        state.modulePresetMenu = -1;
        state.deleteModuleConfirm = false;
        return true;
      }
      if (x >= mx + 204 && x <= mx + 284 && y >= by && y <= by + 26) {
        state.modulePresetMenu = -1;
        state.deleteModuleConfirm = false;
        return true;
      }
      state.modulePresetMenu = -1;
      state.deleteModuleConfirm = false;
      return true;
    }
    for (const Widget& w : widgets_) {
      if (w.kind == Kind::BrowserTab && hitWidget(w, x, y)) {
        state.browserCategory = static_cast<int>(w.value);
        state.browserScroll = 0;
        state.browserPresetFilter = false;
        state.browserQuery.clear();
        state.browserSearch = false;
        return true;
      }
      if (w.kind == Kind::BrowserSearch && hitWidget(w, x, y)) {
        state.browserSearch = true;
        state.browserFresh = true;
        return true;
      }
      if (w.kind == Kind::BrowserScroll && hitWidget(w, x, y)) {
        const int maxScroll = std::max(
            0, browserOptionCount(state.browserCategory, state.browserQuery,
                                  presetFilterTypeFor(shared, state)) -
                   browserVisibleCount(state.browserQuery));
        state.browserScroll = std::clamp(
            state.browserScroll + static_cast<int>(w.value), 0, maxScroll);
        return true;
      }
      if (w.kind != Kind::BrowserOption || !hitWidget(w, x, y)) continue;
      if (w.value <= -2000) {
        const int idx = -2000 - w.value;
        if (idx >= 0 && idx < static_cast<int>(modulePresets_.size())) {
          const ModulePreset& p = modulePresets_[static_cast<size_t>(idx)];
          const int slot = state.browserSlot;
          shared.set(slotParam(slot, kSlotOffsetType),
                     static_cast<double>(p.type));
          for (int k = 0; k < 12; ++k) {
            shared.set(slotParam(slot, kSlotOffsetC0 + k),
                       static_cast<double>(p.controls[static_cast<size_t>(k)]));
          }
        }
      } else if (w.value <= -1000) {
        const int idx = -1000 - w.value;
        if (idx >= 0 && idx < kFactoryModulePresetCount) {
          const FactoryModulePreset& p = kFactoryModulePresets[idx];
          const int slot = state.browserSlot;
          shared.set(slotParam(slot, kSlotOffsetType),
                     static_cast<double>(p.type));
          for (int k = 0; k < 12; ++k) {
            shared.set(slotParam(slot, kSlotOffsetC0 + k),
                       static_cast<double>(p.controls[static_cast<size_t>(k)]));
          }
        }
      } else if (w.value >= 0) {
        applyModuleDefaults(shared, state.browserSlot,
                            static_cast<int>(w.value));
      }
      state.browserSlot = -1;
      state.browserSearch = false;
      state.browserQuery.clear();
      state.browserPresetFilter = false;
      state.presetDirty = true;
      return true;
    }
    state.browserSlot = -1;
    state.browserSearch = false;
    state.browserQuery.clear();
    state.browserPresetFilter = false;
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
      case Kind::ModulePresetButton:
        state.browserSlot = static_cast<int>(w.value);
        state.browserCategory = 6;
        state.browserScroll = 0;
        state.browserQuery.clear();
        state.browserSearch = false;
        state.browserPresetFilter = true;
        state.presetOpen = 0;
        return true;
      case Kind::ModuleName:
      case Kind::ModuleDragHandle:
        beginSlotDrag(state, static_cast<int>(w.value), x, y);
        return true;
      case Kind::BrowserOption:
      case Kind::BrowserTab:
      case Kind::BrowserScroll:
      case Kind::BrowserSearch:
        break;
      case Kind::ModuleToggle:
        shared.set(w.param, shared.get(w.param) > 0.5 ? 0.0 : 1.0);
        state.presetDirty = true;
        return true;
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
      case Kind::Knob: {
        const auto now = std::chrono::steady_clock::now();
        if (lastClickParam_ == static_cast<int>(w.param) &&
            now - lastClickTime_ <= std::chrono::milliseconds(350)) {
          double reset = kParamSpecs[w.param].def;
          if (w.param >= P_SLOT0_TYPE) {
            const int slot =
                static_cast<int>((w.param - P_SLOT0_TYPE) / kSlotStride);
            const int offset =
                static_cast<int>((w.param - P_SLOT0_TYPE) % kSlotStride);
            if (slot >= 0 && slot < kSlotCount && offset >= kSlotOffsetC0) {
              const int control = offset - kSlotOffsetC0;
              const int type = static_cast<int>(
                  shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
              reset = defaultControlValue(moduleDesc(type)->id, control);
            }
          }
          shared.set(w.param, reset);
          state.presetDirty = true;
          lastClickParam_ = -1;
          dragParam_ = -1;
          return true;
        }
        lastClickParam_ = static_cast<int>(w.param);
        lastClickTime_ = now;
        dragParam_ = static_cast<int>(w.param);
        dragSteps_ = w.steps;
        dragStartY_ = y;
        dragStartValue_ = shared.get(w.param);
        return false;
      }
    }
  }
  return false;
}

bool Panel::pointerMove(Shared& shared, PanelState& state, double x, double y,
                        bool fine) {
  if (dragSlot_ >= 0) {
    if (!slotDragging_) {
      const double dx = x - dragStartX_;
      const double dy = y - dragStartY_;
      if (dx * dx + dy * dy > 36.0) {
        slotDragging_ = true;
      }
    }
    if (slotDragging_) {
      dragX_ = x;
      dragY_ = y;
      const int target = slotAtX(x);
      if (target != dragSlot_) {
        moveSlot(shared, dragSlot_, target);
        dragSlot_ = target;
        dropSlot_ = target;
        state.selectedSlot = target;
        state.presetDirty = true;
      } else {
        dropSlot_ = target;
      }
    }
    return true;
  }
  if (dragParam_ < 0) return false;
  const uint32_t param = static_cast<uint32_t>(dragParam_);
  const ParamSpec& spec = kParamSpecs[param];
  const double perPixel = spec.stepped ? 1.0 / 18.0
                                       : (spec.max - spec.min) / (fine ? 800.0 : 200.0);
  double v = dragStartValue_ + (dragStartY_ - y) * perPixel;
  if (dragSteps_ > 1) {
    v = std::round(v * (dragSteps_ - 1)) / (dragSteps_ - 1);
  }
  shared.set(param, v);
  state.presetDirty = true;
  return true;
}

void Panel::pointerUp(Shared& shared, PanelState& state) {
  (void)shared;
  if (dragSlot_ >= 0) {
    if (!slotDragging_) {
      state.selectedSlot = dragSlot_;
      state.browserSlot = dragSlot_;
      state.browserScroll = 0;
      state.browserQuery.clear();
      state.browserSearch = false;
      state.browserPresetFilter = false;
      state.presetOpen = 0;
    }
    dragSlot_ = -1;
    dropSlot_ = -1;
    slotDragging_ = false;
  }
  dragParam_ = -1;
}

bool Panel::wheel(Shared& shared, PanelState& state, double x, double y, double dy) {
  if (state.presetOpen) {
    const int maxScroll =
        std::max(0, presetCount() - presetRowsFor(presetCount()));
    state.presetScroll = std::clamp(
        state.presetScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
    return true;
  }
  if (state.aboutOpen) {
    const int maxScroll = aboutMaxScroll();
    state.aboutScroll = std::clamp(
        state.aboutScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
    return true;
  }
  if (state.browserSlot >= 0) {
    const int maxScroll = std::max(
        0, browserOptionCount(state.browserCategory, state.browserQuery,
                              presetFilterTypeFor(shared, state)) -
               browserVisibleCount(state.browserQuery));
    state.browserScroll = std::clamp(
        state.browserScroll - (dy > 0 ? 1 : -1), 0, maxScroll);
    return true;
  }
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::Knob || !hitWidget(w, x, y)) continue;
    const ParamSpec& spec = kParamSpecs[w.param];
    if (w.steps > 1) {
      const double step = 1.0 / (w.steps - 1);
      const double cur = shared.get(w.param);
      double v = cur + (dy > 0 ? step : -step);
      v = std::round(v / step) * step;
      shared.set(w.param, v);
    } else if (spec.stepped) {
      shared.set(w.param, shared.get(w.param) - (dy > 0 ? 1.0 : -1.0));
    } else {
      shared.set(w.param, shared.get(w.param) - dy * (spec.max - spec.min) * 0.01);
    }
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
    state.presetOpen = 0;
    return true;
  }
  if (x >= 366 && x <= 402) {
    const int next = std::min(presetCount() - 1, state.presetIndex + 1);
    applyPreset(shared, state, next);
    state.presetOpen = 0;
    return true;
  }
  if (x >= 110 && x <= 365) {
    state.presetOpen = state.presetOpen ? 0 : 1;
    state.browserSlot = -1;
    state.deletePresetConfirm = false;
    if (state.presetOpen) {
      const int rows = presetRowsFor(presetCount());
      const int maxScroll = std::max(0, presetCount() - rows);
      state.presetScroll = state.presetIndex >= 0
                               ? std::clamp(state.presetIndex - rows + 1, 0,
                                            maxScroll)
                               : 0;
    }
    return true;
  }
  return x >= 72 && x <= 402;
}

int Panel::presetCount() const {
  return kPresetCount + static_cast<int>(userPresets_.size());
}

const char* Panel::presetName(int index) const {
  if (index >= 0 && index < kPresetCount) return kPresets[index].name;
  const int user = index - kPresetCount;
  if (user >= 0 && user < static_cast<int>(userPresets_.size())) {
    return userPresets_[static_cast<size_t>(user)].name.c_str();
  }
  return "Init";
}

void Panel::applyPreset(Shared& shared, PanelState& state, int index) const {
  if (index < 0 || index >= presetCount()) return;
  if (index < kPresetCount) {
    for (int slot = 0; slot < kSlotCount; ++slot) {
      shared.set(slotParam(slot, kSlotOffsetBypass), 0.0);
      shared.set(slotParam(slot, kSlotOffsetMix), 100.0);
      shared.set(slotParam(slot, kSlotOffsetGain), 0.0);
    }
    const Preset& p = kPresets[index];
    for (int i = 0; i < p.count; ++i) {
      const PresetValue& v = p.values[i];
      if (v.param >= 0 && v.param < static_cast<int>(P_COUNT))
        shared.set(static_cast<uint32_t>(v.param), v.value);
    }
  } else {
    const UserPreset& p =
        userPresets_[static_cast<size_t>(index - kPresetCount)];
    const int n = std::min(static_cast<int>(p.values.size()),
                           static_cast<int>(P_COUNT));
    for (int i = 0; i < n; ++i) {
      shared.set(static_cast<uint32_t>(i), p.values[static_cast<size_t>(i)]);
    }
  }
  state.presetIndex = index;
  state.presetDirty = false;
}

void Panel::exportPreset(const Shared& shared, const PanelState& state) const {
  const int index = state.presetIndex;
  if (index < 0 || index >= presetCount()) return;
  std::vector<double> values;
  values.reserve(P_COUNT);
  for (uint32_t i = 0; i < P_COUNT; ++i) values.push_back(shared.get(i));
  writeUserPreset(userPresetExportDir(), presetName(index), values);
}

bool Panel::importPresetFile(Shared& shared, PanelState& state,
                             const std::string& path) {
  UserPreset preset;
  if (!readUserPresetFile(path, preset)) return false;
  const std::string name = uniquePresetName(userPresetDir(), preset.name);
  if (!writeUserPreset(userPresetDir(), name, preset.values)) return false;
  userPresets_ = loadUserPresets(userPresetDir());
  int found = -1;
  for (int i = 0; i < static_cast<int>(userPresets_.size()); ++i) {
    if (userPresets_[static_cast<size_t>(i)].name == name) {
      found = kPresetCount + i;
      break;
    }
  }
  if (found >= 0) applyPreset(shared, state, found);
  state.presetOpen = 0;
  return found >= 0;
}

bool Panel::importPresetDialog(Shared& shared, PanelState& state) {
  const char* home = std::getenv("HOME");
  std::string dir = home && *home
                        ? std::string(home) + "/Documents/Suite Rack Presets/"
                        : std::string(".");
  std::string cmd =
      "zenity --file-selection --title='Import Suite Rack preset' --filename='" +
      dir + "' 2>/dev/null";
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) return false;
  char buffer[1024] = {};
  const char* got = fgets(buffer, sizeof(buffer), pipe);
  pclose(pipe);
  if (!got) return false;
  std::string path(buffer);
  while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) {
    path.pop_back();
  }
  return !path.empty() && importPresetFile(shared, state, path);
}

void Panel::beginTextEntry(Shared& shared, PanelState& state, double x,
                           double y) {
  if (state.browserSlot >= 0) {
    for (const Widget& w : widgets_) {
      if (w.kind != Kind::BrowserOption || !hitWidget(w, x, y)) continue;
      if (w.value <= -2000) {
        const int idx = -2000 - w.value;
        if (idx >= 0 && idx < static_cast<int>(modulePresets_.size())) {
          state.modulePresetMenu = idx;
          state.deleteModuleConfirm = false;
        }
      }
      return;
    }
  }
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::Knob || !hitWidget(w, x, y)) continue;
    state.textParam = static_cast<int>(w.param);
    state.textBuffer = formatParamValue(shared, w.param, shared.get(w.param));
    state.textFresh = true;
    return;
  }
  for (const Widget& w : widgets_) {
    if (w.kind != Kind::ModuleName || !hitWidget(w, x, y)) continue;
    const int slot = static_cast<int>(w.value);
    const int type =
        static_cast<int>(shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
    if (type <= 0) break;
    state.saveModule = true;
    state.saveModuleSlot = slot;
    state.textParam = -1;
    state.textBuffer = std::string(slotModuleName(type)) + " Setting";
    state.textFresh = true;
    return;
  }
  state.textParam = -1;
  state.textBuffer.clear();
}

bool Panel::textKey(Shared& shared, PanelState& state, int key, char ch) {
  if (state.renameModulePreset) {
    if (key == kTextEscape) {
      state.renameModulePreset = false;
      state.renameModuleIndex = -1;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      const int index = state.renameModuleIndex;
      if (index >= 0 && index < static_cast<int>(modulePresets_.size())) {
        const ModulePreset old = modulePresets_[static_cast<size_t>(index)];
        const std::string newName = sanitizePresetName(state.textBuffer);
        if (!newName.empty() &&
            renameModulePreset(modulePresetDir(), old.name, newName, old.type,
                               old.controls)) {
          modulePresets_ = loadModulePresets(modulePresetDir());
          state.renameModulePreset = false;
          state.renameModuleIndex = -1;
          state.textBuffer.clear();
        }
      }
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.renamePreset) {
    if (key == kTextEscape) {
      state.renamePreset = false;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      const int index = state.presetIndex;
      if (index >= kPresetCount &&
          index < kPresetCount + static_cast<int>(userPresets_.size())) {
        const std::string oldName = presetName(index);
        const auto& values =
            userPresets_[static_cast<size_t>(index - kPresetCount)].values;
        const std::string newName = sanitizePresetName(state.textBuffer);
        if (!newName.empty() &&
            renameUserPreset(userPresetDir(), oldName, newName, values)) {
          userPresets_ = loadUserPresets(userPresetDir());
          for (int i = 0; i < static_cast<int>(userPresets_.size()); ++i) {
            if (userPresets_[static_cast<size_t>(i)].name == newName) {
              state.presetIndex = kPresetCount + i;
              break;
            }
          }
          state.renamePreset = false;
          state.textBuffer.clear();
        }
      }
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.presetOpen && key == kTextEscape) {
    state.presetOpen = 0;
    return true;
  }
  if (state.saveModule) {
    if (key == kTextEscape) {
      state.saveModule = false;
      state.saveModuleSlot = -1;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      const int slot = state.saveModuleSlot;
      if (slot >= 0 && slot < kSlotCount) {
        const int type =
            static_cast<int>(shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
        if (type > 0) {
          std::array<float, 12> controls{};
          for (int k = 0; k < 12; ++k) {
            controls[static_cast<size_t>(k)] = static_cast<float>(
                shared.get(slotParam(slot, kSlotOffsetC0 + k)));
          }
          if (saveModulePreset(modulePresetDir(), state.textBuffer, type,
                               controls)) {
            modulePresets_ = loadModulePresets(modulePresetDir());
          }
        }
      }
      state.saveModule = false;
      state.saveModuleSlot = -1;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.savePreset) {
    if (key == kTextEscape) {
      state.savePreset = false;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextEnter) {
      const std::string name = sanitizePresetName(state.textBuffer);
      if (!name.empty() &&
          saveUserPreset(userPresetDir(), name, shared)) {
        userPresets_ = loadUserPresets(userPresetDir());
        for (int i = 0; i < static_cast<int>(userPresets_.size()); ++i) {
          if (userPresets_[static_cast<size_t>(i)].name == name) {
            state.presetIndex = kPresetCount + i;
            state.presetDirty = false;
            break;
          }
        }
      }
      state.savePreset = false;
      state.textBuffer.clear();
      return true;
    }
    if (key == kTextBackspace) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      } else if (!state.textBuffer.empty()) {
        state.textBuffer.pop_back();
      }
      return true;
    }
    if (ch >= 32 && ch < 127 && state.textBuffer.size() < 60) {
      if (state.textFresh) {
        state.textBuffer.clear();
        state.textFresh = false;
      }
      state.textBuffer.push_back(ch);
    }
    return true;
  }
  if (state.browserSearch) {
    if (key == kTextEscape) {
      state.browserSearch = false;
      state.browserQuery.clear();
      state.browserScroll = 0;
      return true;
    }
    if (key == kTextEnter) {
      state.browserSearch = false;
      return true;
    }
    if (key == kTextBackspace) {
      if (state.browserFresh) {
        state.browserQuery.clear();
        state.browserFresh = false;
      } else if (!state.browserQuery.empty()) {
        state.browserQuery.pop_back();
      }
      state.browserScroll = 0;
      return true;
    }
    if (ch >= 32 && ch < 127 && state.browserQuery.size() < 32) {
      if (state.browserFresh) {
        state.browserQuery.clear();
        state.browserFresh = false;
      }
      state.browserQuery.push_back(ch);
      state.browserScroll = 0;
    }
    return true;
  }
  if (state.textParam < 0) return false;
  if (key == kTextEscape) {
    state.textParam = -1;
    state.textBuffer.clear();
    return true;
  }
  if (key == kTextEnter) {
    double value = 0.0;
    if (parseParamValue(shared, static_cast<uint32_t>(state.textParam),
                        state.textBuffer, &value)) {
      shared.set(static_cast<uint32_t>(state.textParam), value);
      state.presetDirty = true;
    }
    state.textParam = -1;
    state.textBuffer.clear();
    return true;
  }
  if (key == kTextBackspace) {
    if (state.textFresh) {
      state.textBuffer.clear();
      state.textFresh = false;
    } else if (!state.textBuffer.empty()) {
      state.textBuffer.pop_back();
    }
    return true;
  }
  if (ch >= 32 && ch < 127 && state.textBuffer.size() < 32) {
    if (state.textFresh) {
      state.textBuffer.clear();
      state.textFresh = false;
    }
    state.textBuffer.push_back(ch);
  }
  return true;
}

void Panel::drawTextEntry(cairo_t* cr, const PanelState& state) const {
  if (state.textParam < 0 && !state.savePreset && !state.saveModule &&
      !state.renamePreset && !state.renameModulePreset) {
    return;
  }
  const Widget* target = nullptr;
  if (state.textParam >= 0) {
    for (const Widget& w : widgets_) {
      if (w.kind == Kind::Knob &&
          static_cast<int>(w.param) == state.textParam) {
        target = &w;
        break;
      }
    }
  }
  const double w = 480.0, h = 44.0;
  const double x = (kPanelWidth - w) * 0.5;
  const double y = 54.0;

  setRgb(cr, {0.02, 0.02, 0.03}, 0.60);
  roundRect(cr, x + 3, y + 4, w, h, 7);
  cairo_fill(cr);
  setRgb(cr, {0.10, 0.10, 0.11});
  roundRect(cr, x, y, w, h, 7);
  cairo_fill(cr);
  setRgb(cr, {0.42, 0.86, 0.52});
  cairo_set_line_width(cr, 1.4);
  roundRect(cr, x + 0.7, y + 0.7, w - 1.4, h - 1.4, 7);
  cairo_stroke(cr);

  const char* name =
      state.renamePreset
          ? "RENAME PRESET"
          : (state.renameModulePreset
                 ? "RENAME MODULE PRESET"
                 : (state.savePreset
                        ? "SAVE PRESET"
                        : (state.saveModule
                               ? "SAVE MODULE PRESET"
                               : (target ? target->label : "Value"))));
  text(cr, name, x + 14, y + 16, 8.5, 0, 0.72, 0.78, 0.74);
  text(cr, (state.savePreset || state.saveModule) ? "TYPE NAME" : "TYPE VALUE",
       x + 14, y + 33, 7.0, 0, 0.50, 0.56, 0.52);
  const double valueX = x + 150.0;
  const double valueY = y + 34.0;
  cairo_save(cr);
  cairo_rectangle(cr, x + 138, y + 6, w - 210, h - 12);
  cairo_clip(cr);
  text(cr, state.textBuffer.c_str(), valueX, valueY, 16, 0, 0.94, 0.96,
       0.94);
  if (cursorVisible()) {
    cairo_set_font_face(cr, font_);
    cairo_set_font_size(cr, 16);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, state.textBuffer.c_str(), &extents);
    text(cr, "_", valueX + extents.x_advance + 1.0, valueY, 16, 0, 0.94,
         0.96, 0.94);
  }
  cairo_restore(cr);
  text(cr, "ENTER", x + w - 14, y + 16, 7.0, 2, 0.50, 0.56, 0.52);
}

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
