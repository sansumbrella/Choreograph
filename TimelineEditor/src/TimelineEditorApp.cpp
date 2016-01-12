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
  for (auto i = 0; i < channel.keys().size(); i += 1) {

    ui::SliderFloat(("Value " + to_string(i)).c_str(), &channel.mutableKeys()[i].value, 0.0f, 100.0f);

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
}

void drawChannel(ch::Channel<float> &channel) {
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
  for (auto &curve: channel.curves()) {
    curve.solve(0.5f);
  }

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
