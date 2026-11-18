// Suite Rack — panel.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cairo.h>

#include <chrono>

#include <string>
#include <vector>

#include "params.hpp"
#include "user_presets.hpp"

namespace rack {

constexpr int kPanelWidth = 1040;
constexpr int kPanelHeight = 720;

struct PanelState {
  int selectedSlot = 0;
  int browserSlot = -1;
  int presetIndex = -1;
  bool presetDirty = false;
  int presetOpen = 0;
  int presetScroll = 0;
  int browserCategory = 0;
  int browserScroll = 0;
  std::string browserQuery;
  bool browserSearch = false;
  bool browserFresh = true;
  bool browserPresetFilter = false;
  bool aboutOpen = false;
  int aboutScroll = 0;
  int textParam = -1;
  std::string textBuffer;
  bool textFresh = true;
  bool savePreset = false;
  bool saveModule = false;
  int saveModuleSlot = -1;
  bool renamePreset = false;
  bool deletePresetConfirm = false;
  bool renameModulePreset = false;
  int renameModuleIndex = -1;
  int modulePresetMenu = -1;
  bool deleteModuleConfirm = false;
  float inLevelL = -120.0f;
  float inLevelR = -120.0f;
  float outLevelL = -120.0f;
  float outLevelR = -120.0f;
  float slotGrDb[kSlotCount] = {};
};

class Panel {
public:
  Panel();
  ~Panel();
  Panel(const Panel&) = delete;
  Panel& operator=(const Panel&) = delete;

  void loadFont(const std::string& dir);
  void tick(const Shared& shared, PanelState& state) const;
  void draw(cairo_t* cr, Shared& shared, const PanelState& state);

  bool pointerDown(Shared& shared, PanelState& state, double x, double y,
                   bool fine = false);
  bool pointerMove(Shared& shared, PanelState& state, double x, double y,
                   bool fine = false);
  void pointerUp(Shared& shared, PanelState& state);
  bool handlePresetClick(Shared& shared, PanelState& state, double x, double y);
  void applyPreset(Shared& shared, PanelState& state, int index) const;
  int presetCount() const;
  const char* presetName(int index) const;
  bool wheel(Shared& shared, PanelState& state, double x, double y, double dy);
  void beginTextEntry(Shared& shared, PanelState& state, double x, double y);
  static constexpr int kTextEnter = 1;
  static constexpr int kTextEscape = 2;
  static constexpr int kTextBackspace = 3;
  bool textKey(Shared& shared, PanelState& state, int key, char ch);
  bool dragging() const { return dragParam_ >= 0; }

  struct WidgetRect {
    uint32_t param = 0;
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    double r = 0.0;
    const char* label = "";
    bool knob = false;
    bool toggle = false;
  };
  std::vector<WidgetRect> widgetRects() const;

private:
  enum class Kind { Knob, Pill, Button, SlotCard, ModuleButton, ModelButton, ModuleToggle, Power, ModuleName, ModulePresetButton, ModuleDragHandle, BrowserOption, BrowserTab, BrowserScroll, BrowserSearch };
  struct Widget {
    Kind kind = Kind::Knob;
    uint32_t param = 0;
    double value = 0.0;
    double x = 0.0, y = 0.0, w = 0.0, h = 0.0, r = 0.0;
    int steps = 0;
    const char* label = "";
    const char* subLabel = nullptr;
    std::vector<const char*> scale;
    bool labelBelow = false;
    bool compact = false;
    int styleSlot = -1;
    int styleType = -1;
    int moduleControl = -1;
  };

  void layout(const Shared& shared, const PanelState& state);
  void drawHeader(cairo_t* cr, const Shared& shared, const PanelState& state) const;
  void drawRack(cairo_t* cr, const Shared& shared, const PanelState& state) const;
  void drawBrowser(cairo_t* cr, const Shared& shared, const PanelState& state) const;
  void drawWidget(cairo_t* cr, const Widget& w, const Shared& shared,
                  const PanelState& state) const;
  void drawKnob(cairo_t* cr, const Widget& w, const Shared& shared,
                double value) const;
  void drawPill(cairo_t* cr, const Widget& w, bool on, const char* label) const;
  void drawButton(cairo_t* cr, const Widget& w, bool lit) const;
  void drawLevelMeter(cairo_t* cr, double x, double y, double w, double h,
                      double dbL, double dbR) const;
  void beginSlotDrag(PanelState& state, int slot, double x, double y);
  void drawDragGhost(cairo_t* cr, const Shared& shared) const;
  void drawAbout(cairo_t* cr, const Shared& shared,
                 const PanelState& state) const;
  void drawPresetList(cairo_t* cr, const Shared& shared,
                      const PanelState& state) const;
  void drawBrowserMenu(cairo_t* cr, const PanelState& state) const;
  void drawTextEntry(cairo_t* cr, const PanelState& state) const;
  std::vector<UserPreset> userPresets_;
  std::vector<ModulePreset> modulePresets_;
  double browserHeight(int category, const std::string& query,
                       int presetFilterType) const;
  double browserY(int category, const std::string& query,
                  int presetFilterType) const;
  int browserOptionCount(int category, const std::string& query,
                         int presetFilterType) const;
  int aboutMaxScroll() const;
  int slotAtX(double x) const;
  void moveSlot(Shared& shared, int from, int to);
  void drawGrMeter(cairo_t* cr, double x, double y, double w, double h,
                   double grDb) const;
  void text(cairo_t* cr, const char* s, double x, double y, double size,
            int align, double r, double g, double b) const;
  bool hitWidget(const Widget& w, double x, double y) const;

  std::vector<Widget> widgets_;
  int dragParam_ = -1;
  int dragSlot_ = -1;
  int dropSlot_ = -1;
  bool slotDragging_ = false;
  double dragStartX_ = 0.0;
  double dragStartY_ = 0.0;
  double dragX_ = 0.0;
  double dragY_ = 0.0;
  int lastClickParam_ = -1;
  std::chrono::steady_clock::time_point lastClickTime_{};
  double dragStartValue_ = 0.0;
  int dragSteps_ = 0;
  cairo_surface_t* grain_ = nullptr;
  void* ft_ = nullptr;
  void* ftFace_ = nullptr;
  cairo_font_face_t* font_ = nullptr;
};

} // namespace rack
