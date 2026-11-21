// Suite Rack - Cocoa view host
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <memory>

namespace rack {

class Shared;
struct PanelState;
class Panel;

// Hosts the cairo Panel inside an NSView.  Suite Rack's panel draws straight
// from Shared/PanelState, so there is no value-sync callback: the view just
// ticks the panel and repaints.
class CocoaView {
public:
  CocoaView();
  ~CocoaView();
  CocoaView(const CocoaView&) = delete;
  CocoaView& operator=(const CocoaView&) = delete;

  void setContent(Panel* panel, Shared* shared, PanelState* state);
  void attach(void* parentNsView);
  void detach();
  void resize(int width, int height);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace rack
