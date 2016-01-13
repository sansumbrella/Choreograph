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

  auto outer_size = vec2(ui::GetWindowWidth(), 100.0f);
  auto padding = vec2(20, 10);
  auto top_left = vec2(0);
  auto bottom_right = outer_size - padding;
  auto range = vec2(-100.0f, 100.0f);
  auto color = vec4(1.0f,1.0f,0.4f,1.0f);
  auto color32 = ImColor(color);
  auto background_color = ImColor(vec4(vec3(0.1f), 1.0f));
  auto position = vec2(ui::GetCursorScreenPos());

  draw_list->AddRectFilled(position + top_left, position + bottom_right, background_color);
  for (auto &key: channel.keys()) {
    auto x = key.time / channel.duration();
    auto y = (key.value - range.x) / (range.y - range.x);
    auto pos = mix(top_left, bottom_right, vec2(x, y)) + position;

    draw_list->AddCircle(pos, 4.0, color32);
  }
  ImGui::Dummy(outer_size);
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
