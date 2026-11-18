// Suite Rack — panel interaction tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cairo.h>
#include <cmath>
#include <cstdio>
#include <string>

#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "ui/panel.hpp"

using namespace rack;

namespace {
int fails = 0;
void check(bool ok, const char* what) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) ++fails;
}

bool clickParam(Panel& panel, Shared& s, PanelState& state, uint32_t param) {
  for (const auto& r : panel.widgetRects()) {
    if (r.param != param) continue;
    const double x = r.x + r.w * 0.5;
    const double y = r.y + r.h * 0.5;
    panel.pointerDown(s, state, x, y, false);
    panel.pointerUp();
    return true;
  }
  return false;
}

bool clickLabel(Panel& panel, Shared& s, PanelState& state,
                const char* label) {
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) != label) continue;
    panel.pointerDown(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, false);
    panel.pointerUp();
    return true;
  }
  return false;
}
} // namespace

int main() {
  cairo_surface_t* surf =
      cairo_image_surface_create(CAIRO_FORMAT_RGB24, kPanelWidth, kPanelHeight);
  cairo_t* cr = cairo_create(surf);
  Shared s;
  initializeRack(s);
  Panel panel;
  PanelState state;
  panel.draw(cr, s, state);

  check(state.selectedSlot == 0, "starts on slot 1");
  check(s.get(P_SLOT0_TYPE) == static_cast<double>(ModuleId::Eq4K),
        "slot 1 starts as 4K EQ");
  check(s.get(P_SLOT1_TYPE) == static_cast<double>(ModuleId::Comp76Blue),
        "slot 2 starts as 76 Blue");

  // Click the second column's module name to open its browser.
  panel.pointerDown(s, state, 250, 134, false);
  panel.pointerUp();
  check(state.selectedSlot == 1 && state.browserSlot == 1,
        "clicking the second module name opens its browser");
  panel.draw(cr, s, state);

  check(clickLabel(panel, s, state, "OFF"), "browser offers Off");
  check(s.get(P_SLOT1_TYPE) == static_cast<double>(ModuleId::Off),
        "Off module clears the slot");

  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 250, 134, false);
  panel.pointerUp();
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "COMP"), "browser category tab is clickable");
  check(state.browserCategory == 2, "COMP tab filters the browser");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "VCA"), "filtered browser offers VCA");
  check(s.get(P_SLOT1_TYPE) == static_cast<double>(ModuleId::CompVCA),
        "VCA module selects into the slot");

  // Slot 3: COLOUR category and the Valve module.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 441, 134, false);
  panel.pointerUp();
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "COLOUR"),
        "COLOUR category tab is clickable");
  check(state.browserCategory == 3, "COLOUR tab filters the browser");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Valve"), "filtered browser offers Valve");
  check(s.get(P_SLOT2_TYPE) == static_cast<double>(ModuleId::ColourValve),
        "Valve module selects into slot 3");

  // Slot 4: DYN category and the Gate module.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 596, 134, false);
  panel.pointerUp();
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "DYN"), "DYN category tab is clickable");
  check(state.browserCategory == 4, "DYN tab filters the browser");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Gate"), "filtered browser offers Gate");
  check(s.get(P_SLOT3_TYPE) == static_cast<double>(ModuleId::DynGate),
        "Gate module selects into slot 4");

  // Slot 5: Bleed Out and its stepped profile control.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 750, 134, false);
  panel.pointerUp();
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "DYN"), "DYN tab works on slot 5");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Bleed Out"),
        "filtered browser offers Bleed Out");
  check(s.get(P_SLOT4_TYPE) == static_cast<double>(ModuleId::BleedOut),
        "Bleed Out selects into slot 5");

  const uint32_t profileParam = slotParam(4, kSlotOffsetC0 + 0);
  s.set(profileParam, 0.0);
  panel.draw(cr, s, state);
  bool wheeled = false;
  for (const auto& r : panel.widgetRects()) {
    if (r.param != profileParam) continue;
    wheeled = panel.wheel(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, 1.0);
    break;
  }
  check(wheeled, "Bleed Out profile knob is wheelable");
  check(std::fabs(s.get(profileParam) - 1.0 / 7.0) < 1e-6,
        "stepped profile snaps to the next profile");

  const uint32_t ghostOnParam = slotParam(4, kSlotOffsetC0 + 4);
  check(s.get(ghostOnParam) == 1.0, "ghost starts enabled");
  check(clickParam(panel, s, state, ghostOnParam),
        "ghost toggle is clickable");
  check(s.get(ghostOnParam) == 0.0, "ghost toggle turns ghost off");

  // Slot 6: Peak Limiter.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 906, 134, false);
  panel.pointerUp();
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Peak Limiter"),
        "filtered browser offers Peak Limiter");
  check(s.get(P_SLOT5_TYPE) == static_cast<double>(ModuleId::Limiter),
        "Peak Limiter selects into slot 6");
  panel.draw(cr, s, state);
  bool limiterControls = true;
  for (int k = 0; k < 4; ++k) {
    bool found = false;
    for (const auto& r : panel.widgetRects()) {
      if (r.param == slotParam(5, kSlotOffsetC0 + k)) found = true;
    }
    limiterControls = limiterControls && found;
  }
  check(limiterControls, "Peak Limiter exposes all four controls");

  // Bypass pill for slot 2.
  panel.draw(cr, s, state);
  check(clickParam(panel, s, state, P_SLOT1_BYPASS),
        "bypass pill is clickable");
  check(s.get(P_SLOT1_BYPASS) == 1.0, "bypass pill toggles the selected slot");

  // Global power.
  check(clickParam(panel, s, state, P_POWER), "power button is clickable");
  check(s.get(P_POWER) == 0.0, "power button toggles global power");

  // The generic slot controls exist for the active module.
  panel.draw(cr, s, state);
  bool gainFound = false;
  for (const auto& r : panel.widgetRects()) {
    if (r.param == slotParam(0, kSlotOffsetGain)) gainFound = true;
  }
  check(gainFound, "per-slot gain is present");

  // Double-clicking a knob resets it to the active module's default.
  const uint32_t lowParam = slotParam(0, kSlotOffsetC0 + kCtlLow);
  s.set(lowParam, 0.90);
  panel.draw(cr, s, state);
  check(clickParam(panel, s, state, lowParam), "EQ low knob is clickable");
  check(clickParam(panel, s, state, lowParam),
        "EQ low knob accepts a double-click");
  check(std::fabs(s.get(lowParam) - 0.60) < 1e-6,
        "double-click resets the control to its module default");

  check(!panel.widgetRects().empty(), "panel reports widget geometry");

  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  std::printf("%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
