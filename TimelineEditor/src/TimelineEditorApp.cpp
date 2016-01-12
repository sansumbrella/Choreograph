#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderImGui.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui_user.h"

#include "Choreograph.h"
#include "Channel.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;


bool drawChannelGui(const ch::Channel<float> &channel) {
  using namespace ImGui;
  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems)
      return false;

  auto label = "channel";
  ImGuiState& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(label);
  const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

  auto size_arg = ImVec2(0, 0);
  ImVec2 pos = window->DC.CursorPos;
  if (style.FramePadding.y < window->DC.CurrentLineTextBaseOffset)
      pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
  ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

  const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
  ItemSize(bb, style.FramePadding.y);
  if (!ItemAdd(bb, &id))
      return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held);

  // Render
  const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
  RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
  RenderTextClipped(bb.Min, bb.Max, label, NULL, &label_size, ImGuiAlign_Center | ImGuiAlign_VCenter);

  return pressed;
}

void drawChannel(const ch::Channel<float> &channel) {
  auto inner_size = vec2(200, 50);
  auto padding = vec2(10);
  auto outer_size = inner_size + padding * 2.0f;
  gl::ScopedColor color_scope;
  gl::ScopedModelMatrix matrix_scope;

  gl::color(Color::gray(0.5f));
  gl::drawSolidRect(Rectf(vec2(0), outer_size));
  gl::translate(padding);

  auto range = ([&] {
    auto range = vec2(std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
    for (auto &key: channel.keys()) {
      range.x = min(range.x, key.value);
      range.y = max(range.x, key.value);
    }

    return range;
  } ());

  gl::color(Color::gray(1.0f));
  for (auto &key: channel.keys()) {
    auto x = key.time / channel.duration();
    auto y = (key.value - range.x) / (range.y - range.x);
    auto pos = vec2(x, y) * inner_size;

    gl::drawSolidCircle(pos, 8.0f);
  }
}

class TimelineEditorApp : public App {
public:
  void setup() override;
  void mouseDown( MouseEvent event ) override;
  void update() override;
  void draw() override;
private:
  ch::Channel<float> _channel;
};

void TimelineEditorApp::setup()
{
	ui::initialize();

  _channel.insertKey(10.0f, 0);
  _channel.insertKey(20.0f, 1);
  _channel.insertKey(30.0f, 3);

  CI_ASSERT(_channel.duration() == 3);
}

void TimelineEditorApp::mouseDown( MouseEvent event )
{
}

void TimelineEditorApp::update()
{
  ui::Text("Hello");
  drawChannelGui(_channel);
}

void TimelineEditorApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
  drawChannel(_channel);
}

CINDER_APP( TimelineEditorApp, RendererGl )
