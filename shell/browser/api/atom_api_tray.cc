// Copyright (c) 2014 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "shell/browser/api/atom_api_tray.h"

#include <string>

#include "base/threading/thread_task_runner_handle.h"
#include "native_mate/constructor.h"
#include "native_mate/dictionary.h"
#include "shell/browser/api/atom_api_menu.h"
#include "shell/browser/browser.h"
#include "shell/common/api/atom_api_native_image.h"
#include "shell/common/native_mate_converters/gfx_converter.h"
#include "shell/common/native_mate_converters/image_converter.h"
#include "shell/common/native_mate_converters/string16_converter.h"
#include "shell/common/node_includes.h"
#include "ui/gfx/image/image.h"

namespace mate {

template <>
struct Converter<electron::TrayIcon::IconType> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     electron::TrayIcon::IconType* out) {
    using IconType = electron::TrayIcon::IconType;
    std::string mode;
    if (ConvertFromV8(isolate, val, &mode)) {
      if (mode == "none") {
        *out = IconType::None;
        return true;
      } else if (mode == "info") {
        *out = IconType::Info;
        return true;
      } else if (mode == "warning") {
        *out = IconType::Warning;
        return true;
      } else if (mode == "error") {
        *out = IconType::Error;
        return true;
      } else if (mode == "custom") {
        *out = IconType::Custom;
        return true;
      }
    }
    return false;
  }
};

}  // namespace mate

namespace electron {

namespace api {

Tray::Tray(v8::Isolate* isolate,
           v8::Local<v8::Object> wrapper,
           mate::Handle<NativeImage> image)
    : tray_icon_(TrayIcon::Create()) {
  SetImage(isolate, image);
  tray_icon_->AddObserver(this);

  InitWith(isolate, wrapper);
}

Tray::~Tray() = default;

// static
mate::WrappableBase* Tray::New(mate::Handle<NativeImage> image,
                               mate::Arguments* args) {
  if (!Browser::Get()->is_ready()) {
    args->ThrowError("Cannot create Tray before app is ready");
    return nullptr;
  }
  return new Tray(args->isolate(), args->GetThis(), image);
}

void Tray::OnClicked(const gfx::Rect& bounds,
                     const gfx::Point& location,
                     int modifiers) {
  EmitWithFlags("click", modifiers, bounds, location);
}

void Tray::OnDoubleClicked(const gfx::Rect& bounds, int modifiers) {
  EmitWithFlags("double-click", modifiers, bounds);
}

void Tray::OnRightClicked(const gfx::Rect& bounds, int modifiers) {
  EmitWithFlags("right-click", modifiers, bounds);
}

void Tray::OnBalloonShow() {
  Emit("balloon-show");
}

void Tray::OnBalloonClicked() {
  Emit("balloon-click");
}

void Tray::OnBalloonClosed() {
  Emit("balloon-closed");
}

void Tray::OnDrop() {
  Emit("drop");
}

void Tray::OnDropFiles(const std::vector<std::string>& files) {
  Emit("drop-files", files);
}

void Tray::OnDropText(const std::string& text) {
  Emit("drop-text", text);
}

void Tray::OnMouseEntered(const gfx::Point& location, int modifiers) {
  EmitWithFlags("mouse-enter", modifiers, location);
}

void Tray::OnMouseExited(const gfx::Point& location, int modifiers) {
  EmitWithFlags("mouse-leave", modifiers, location);
}

void Tray::OnMouseMoved(const gfx::Point& location, int modifiers) {
  EmitWithFlags("mouse-move", modifiers, location);
}

void Tray::OnDragEntered() {
  Emit("drag-enter");
}

void Tray::OnDragExited() {
  Emit("drag-leave");
}

void Tray::OnDragEnded() {
  Emit("drag-end");
}

void Tray::SetImage(v8::Isolate* isolate, mate::Handle<NativeImage> image) {
#if defined(OS_WIN)
  tray_icon_->SetImage(image->GetHICON(GetSystemMetrics(SM_CXSMICON)));
#else
  tray_icon_->SetImage(image->image());
#endif
}

void Tray::SetPressedImage(v8::Isolate* isolate,
                           mate::Handle<NativeImage> image) {
#if defined(OS_WIN)
  tray_icon_->SetPressedImage(image->GetHICON(GetSystemMetrics(SM_CXSMICON)));
#else
  tray_icon_->SetPressedImage(image->image());
#endif
}

void Tray::SetToolTip(const std::string& tool_tip) {
  tray_icon_->SetToolTip(tool_tip);
}

void Tray::SetTitle(const std::string& title) {
#if defined(OS_MACOSX)
  tray_icon_->SetTitle(title);
#endif
}

std::string Tray::GetTitle() {
#if defined(OS_MACOSX)
  return tray_icon_->GetTitle();
#else
  return "";
#endif
}

void Tray::SetIgnoreDoubleClickEvents(bool ignore) {
#if defined(OS_MACOSX)
  tray_icon_->SetIgnoreDoubleClickEvents(ignore);
#endif
}

bool Tray::GetIgnoreDoubleClickEvents() {
#if defined(OS_MACOSX)
  return tray_icon_->GetIgnoreDoubleClickEvents();
#else
  return false;
#endif
}

void Tray::DisplayBalloon(mate::Arguments* args,
                          const mate::Dictionary& options) {
  TrayIcon::BalloonOptions balloon_options;

  if (!options.Get("title", &balloon_options.title) ||
      !options.Get("content", &balloon_options.content)) {
    args->ThrowError("'title' and 'content' must be defined");
    return;
  }

  mate::Handle<NativeImage> icon;
  options.Get("icon", &icon);
  options.Get("iconType", &balloon_options.icon_type);
  options.Get("largeIcon", &balloon_options.large_icon);
  options.Get("noSound", &balloon_options.no_sound);
  options.Get("respectQuietTime", &balloon_options.respect_quiet_time);

  if (!icon.IsEmpty()) {
#if defined(OS_WIN)
    balloon_options.icon = icon->GetHICON(
        GetSystemMetrics(balloon_options.large_icon ? SM_CXICON : SM_CXSMICON));
#else
    balloon_options.icon = icon->image();
#endif
  }

  tray_icon_->DisplayBalloon(balloon_options);
}

void Tray::RemoveBalloon() {
  tray_icon_->RemoveBalloon();
}

void Tray::Focus() {
  tray_icon_->Focus();
}

void Tray::PopUpContextMenu(mate::Arguments* args) {
  mate::Handle<Menu> menu;
  args->GetNext(&menu);
  gfx::Point pos;
  args->GetNext(&pos);
  tray_icon_->PopUpContextMenu(pos, menu.IsEmpty() ? nullptr : menu->model());
}

void Tray::SetContextMenu(v8::Isolate* isolate, mate::Handle<Menu> menu) {
  menu_.Reset(isolate, menu.ToV8());
  tray_icon_->SetContextMenu(menu.IsEmpty() ? nullptr : menu->model());
}

gfx::Rect Tray::GetBounds() {
  return tray_icon_->GetBounds();
}

// static
void Tray::BuildPrototype(v8::Isolate* isolate,
                          v8::Local<v8::FunctionTemplate> prototype) {
  prototype->SetClassName(mate::StringToV8(isolate, "Tray"));
  mate::ObjectTemplateBuilder(isolate, prototype->PrototypeTemplate())
      .MakeDestroyable()
      .SetMethod("setImage", &Tray::SetImage)
      .SetMethod("setPressedImage", &Tray::SetPressedImage)
      .SetMethod("setToolTip", &Tray::SetToolTip)
      .SetMethod("_setTitle", &Tray::SetTitle)
      .SetMethod("_getTitle", &Tray::GetTitle)
      .SetProperty("title", &Tray::GetTitle, &Tray::SetTitle)
      .SetMethod("_setIgnoreDoubleClickEvents",
                 &Tray::SetIgnoreDoubleClickEvents)
      .SetMethod("_getIgnoreDoubleClickEvents",
                 &Tray::GetIgnoreDoubleClickEvents)
      .SetProperty("ignoreDoubleClickEvents", &Tray::GetIgnoreDoubleClickEvents,
                   &Tray::SetIgnoreDoubleClickEvents)
      .SetMethod("displayBalloon", &Tray::DisplayBalloon)
      .SetMethod("removeBalloon", &Tray::RemoveBalloon)
      .SetMethod("focus", &Tray::Focus)
      .SetMethod("popUpContextMenu", &Tray::PopUpContextMenu)
      .SetMethod("setContextMenu", &Tray::SetContextMenu)
      .SetMethod("getBounds", &Tray::GetBounds);
}

}  // namespace api

}  // namespace electron

namespace {

using electron::api::Tray;

void Initialize(v8::Local<v8::Object> exports,
                v8::Local<v8::Value> unused,
                v8::Local<v8::Context> context,
                void* priv) {
  v8::Isolate* isolate = context->GetIsolate();
  Tray::SetConstructor(isolate, base::BindRepeating(&Tray::New));

  mate::Dictionary dict(isolate, exports);
  dict.Set(
      "Tray",
      Tray::GetConstructor(isolate)->GetFunction(context).ToLocalChecked());
}

}  // namespace

NODE_LINKED_MODULE_CONTEXT_AWARE(atom_browser_tray, Initialize)
