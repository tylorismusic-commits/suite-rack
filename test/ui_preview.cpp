// Suite Rack — renders the panel for visual checks
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cairo.h>
#include <cstdio>
#include <string>
#include "params.hpp"
#include "ui/panel.hpp"
using namespace rack;
int main(int argc, char** argv) {
  if (argc < 3) return 2;
  const std::string font = argv[1], out = argv[2];
  Shared s;
  Panel panel;
  panel.loadFont(font);
  PanelState state;
  cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, kPanelWidth, kPanelHeight);
  cairo_t* cr = cairo_create(surf);
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-eq.png").c_str());
  // Comp editor on slot 2.
  state.selectedSlot = 1;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-comp.png").c_str());
  state.browserSlot = 1;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-browser.png").c_str());
  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  std::printf("wrote rack-eq.png and rack-comp.png\n");
  return 0;
}
