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

void drawChannelGui(ch::Channel<float> &channel) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  for (auto i = 0; i < channel.keys().size(); i += 1) {

    ui::SliderFloat(("Value " + to_string(i)).c_str(), &channel.mutableKeys()[i].value, -100.0f, 100.0f);

    if (i > 0 && i < channel.keys().size() - 1) {
      auto *previous = &channel.mutableKeys()[i - 1];
      auto *current = &channel.mutableKeys()[i];
      auto *next = &channel.mutableKeys()[i + 1];
      auto timef = (float)current->time;
      if (ui::SliderFloat(("Time " + to_string(i)).c_str(), &timef, previous->time, next->time)) {
        current->time = timef;
      }
    }
  }

  auto const outer_size = vec2(ui::GetWindowWidth(), 100.0f);
  auto const padding = vec2(20, 10);
  auto const top_left = vec2(0);
  auto const bottom_right = outer_size - padding;
  auto const value_range = vec2(-100.0f, 100.0f);
  auto const time_range = vec2(0, channel.duration());
  auto const cursor_position = vec2(ui::GetCursorScreenPos());
  auto color = vec4(1.0f,1.0f,0.4f,1.0f);
  auto color32 = ImColor(color);
  auto background_color = ImColor(vec4(1.0f, 0.5f, 0.0f, 1.0f));

  auto const value_to_space = [=] (const ci::vec2 &values) {
    auto x = values.x / channel.duration();
    auto y = (values.y - value_range.x) / (value_range.y - value_range.x);
    return mix(top_left, bottom_right, vec2(x, y)) + cursor_position;
  };

  auto const space_to_value = [=] (const ci::vec2 &space) {
    auto normalized = space / (bottom_right - top_left);
    return mix(vec2(time_range.x, value_range.x), vec2(time_range.y, value_range.y), normalized);
  };

  draw_list->AddRectFilled(cursor_position + top_left, cursor_position + bottom_right, background_color);
  auto id = 0;
  for (auto &key: channel.mutableKeys()) {
    auto pos = value_to_space(vec2(key.time, key.value));
    auto radius = 16.0f;

    // Use an invisible button to handle interaction with circles.
    ui::SetCursorScreenPos(pos - vec2(radius));
    ui::PushID("temp_key");
    ui::InvisibleButton("", vec2(radius * 2.0f));
    ui::SetCursorScreenPos(cursor_position);
    if (ui::IsItemHovered()) {
      // Maybe use DragBehavior to handle changing value? Mapping back from space to value
//      ui::DragBehavior(<#const ImRect &frame_bb#>, <#ImGuiID id#>, <#float *v#>, <#float v_speed#>, <#float v_min#>, <#float v_max#>, <#int decimal_precision#>, <#float power#>)
      // Or use is mouse dragging and handle change more directly?
      if (ui::IsMouseDragging()) {
        // Problem: delta is cumulative; need to store starting position, too.
        auto delta = vec2(ui::GetMouseDragDelta());
        auto value_delta = space_to_value(delta);
        console() << "Changing value of " << id << " with deltas: " << delta << ", " << value_delta << " mouse: " << vec2(ui::GetMousePos()) << endl;
        key.value += value_delta.y;
        key.time += value_delta.x;
      }
    }

    ui::PopID();
    id += 1;
    draw_list->AddCircle(pos, radius, color32);
  }
  ui::Dummy(outer_size);
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
}

CINDER_APP( TimelineEditorApp, RendererGl )
