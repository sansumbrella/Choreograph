#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/gl/TextureFont.h"

#include "samples/Samples.h"
#include "pockets/cobweb/CobWeb.h"

using namespace ci;
using namespace ci::app;
using namespace pockets;
using namespace choreograph;
using namespace std;

class SamplesApp : public App {
public:
	void setup() override;
	void update() override;

  void loadSample( int index );
private:
  pk::SceneRef            _current_scene;
  pk::SceneRef            _previous_scene;
  ch::Timeline            _timeline;
  ci::Timer               _timer;
  int                     _scene_index = 0;
  string                  _scene_name;
  gl::TextureFontRef      _title_font;
  pk::cw::RootNode        _gui;
};

void SamplesApp::setup()
{
  auto button_font = Font( "Arial", toPixels( 18.0f ) );

  using namespace pockets;
  auto previous = cobweb::SimpleButton::createLabelButton( "PREVIOUS", button_font );
  previous->setSelectFn( [this] { loadSample( _scene_index - 1 ); } );
  previous->setPosition( vec2( 10, 20 ) );
  previous->setHitPadding( 10.0f, 10.0f );

  auto next = cobweb::SimpleButton::createLabelButton( "NEXT", button_font );
  next->setSelectFn( [this] { loadSample( _scene_index + 1 ); } );
  next->setPosition( previous->getPosition() + vec2( previous->getWidth() + 10.0f, 0.0f ) );
  next->setHitPadding( 10.0f, 10.0f );

  _gui.appendChild( previous );
  _gui.appendChild( next );
  _gui.connect( getWindow() );

  // Draw our app first, so samples show up over top.
  getWindow()->getSignalDraw().connect( 1, [this] {
    gl::clear( Color::black() );
    _gui.deepDraw();
  } );

  loadSample( 0 );
  _timer.start();
}

void SamplesApp::loadSample( int index )
{
  bool do_animate = (index != _scene_index);
  const int start_x = (index < _scene_index) ? - getWindowWidth() : getWindowWidth();
  const int vanish_x = - start_x;

  float cooldown = ( _current_scene && _current_scene->timeline().empty() ) ? 0 : 0.25f;

  if( index < 0 ) { index = SampleList.size() - 1; }
  index %= SampleList.size();

  _scene_index = index;
  _scene_name = SampleNames[_scene_index];

  console() << "Loading Sample: " << _scene_name << endl;

  if( _current_scene && do_animate ) {
    _previous_scene = _current_scene;
    // Decelerate animation of previous scene down to zero.
    _timeline.apply( _previous_scene->getAnimationSpeedOutput() ).then<RampTo>( 0, 0.4f );

    // Slide previous scene off screen after/during deceleration.
    _timeline.apply( _previous_scene->getOffsetOutput() )
      .hold( cooldown )
      .then<RampTo>( vec2( vanish_x, 0.0f ), 0.4f, EaseInQuad() )
      .finishFn( [this] {
        _previous_scene.reset(); // get rid of previous scene
      } );
  }

  _current_scene = SampleList[_scene_index].second();

  _current_scene->setup();
  _current_scene->connect( getWindow() );
  _current_scene->show( getWindow() );

  // animate current on.
  if( do_animate ) {
    _current_scene->setOffset( vec2( start_x, 0.0f ) );
    _current_scene->pause();

    _timeline.apply( _current_scene->getOffsetOutput() ).hold( 0.2f + cooldown ).then<RampTo>( vec2( 0 ), 0.66f, EaseOutQuint() )
    .finishFn( [this] {
      _current_scene->resume();
    } );

  }
}

void SamplesApp::update()
{
  if( _scene_name != SampleNames[_scene_index] ) {
    loadSample( _scene_index );
  }

  ch::Time dt = (Time)_timer.getSeconds();
  _timer.start();
  _timeline.step( dt );
}

CINDER_APP( SamplesApp, RendererGl( RendererGl::Options().msaa( 0 ) ), []( App::Settings *settings ) {
  settings->setWindowSize( 1280, 720 );
  settings->disableFrameRate();
} )
