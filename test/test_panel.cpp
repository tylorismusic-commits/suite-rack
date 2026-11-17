// Suite Rack — panel interaction tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cairo.h>
#include <cmath>
#include <cstdio>
#include "params.hpp"
#include "ui/panel.hpp"
using namespace rack;
namespace {
int fails = 0;
void check(bool ok, const char* what) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) ++fails;
}
} // namespace
int main() {
  cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, kPanelWidth, kPanelHeight);
  cairo_t* cr = cairo_create(surf);
  Shared s;
  Panel panel;
  PanelState state;
  panel.draw(cr, s, state);
  check(state.selectedSlot == 0, "starts on slot 1");
  check(std::fabs(s.get(P_SLOT0_TYPE) - 1.0) < 1e-9, "slot 1 starts as EQ");
  check(std::fabs(s.get(P_SLOT1_TYPE) - 2.0) < 1e-9, "slot 2 starts as Comp");
  // Click the second row's module name to open the module browser.
  panel.pointerDown(s, state, 250, 134, false);
  panel.pointerUp();
  check(state.selectedSlot == 1 && state.browserSlot == 1,
        "clicking the second module name opens its browser");
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 400, 314, false);
  panel.pointerUp();
  check(s.get(P_SLOT1_TYPE) == 0.0, "Off module button clears the slot");
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 250, 134, false);
  panel.pointerUp();
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 600, 314, false);
  panel.pointerUp();
  check(s.get(P_SLOT1_TYPE) == 1.0, "EQ module button selects EQ");
  // Bypass pill for slot 2.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 280, 165, false);
  panel.pointerUp();
  check(s.get(P_SLOT1_BYPASS) == 1.0, "bypass pill toggles the selected slot");
  // Global power.
  panel.pointerDown(s, state, 34, 32, false);
  panel.pointerUp();
  check(s.get(P_POWER) == 0.0, "power button toggles global power");
  check(!panel.widgetRects().empty(), "panel reports widget geometry");
  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  std::printf("%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
