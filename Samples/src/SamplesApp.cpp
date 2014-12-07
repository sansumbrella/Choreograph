#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/gl/TextureFont.h"

#include "samples/Samples.h"

using namespace ci;
using namespace ci::app;
using namespace pockets;
using namespace choreograph;
using namespace std;

class SamplesApp : public AppNative {
public:
  void prepareSettings( Settings *settings ) override;
	void setup() override;
	void update() override;

  void loadSample( int index );
private:
  pk::SceneRef            _current_scene;
  pk::SceneRef            _previous_scene;
  ch::Timeline            _timeline;
  ci::Timer               _timer;
  std::shared_ptr<ch::Control> _cue_control;
  int                     _scene_index = 0;
  string                  _scene_name;
  params::InterfaceGlRef  _params;
  gl::TextureFontRef      _title_font;
};

void SamplesApp::prepareSettings( Settings *settings )
{
  settings->setWindowSize( 1280, 720 );
  settings->enableMultiTouch();
  settings->disableFrameRate();
}

void SamplesApp::setup()
{

  _title_font = gl::TextureFont::create( Font( "Arial Bold", 14.0f ) );

#ifndef CINDER_COCOA_TOUCH
  _params = params::InterfaceGl::create( "Choreograph Samples", ivec2( 240, 100 ) );
  _params->setPosition( ivec2( getWindowWidth() - 250, 10 ) );
  _params->addParam( "Sample", SampleNames, &_scene_index );
  _params->addButton( "Next", [this] { loadSample( _scene_index + 1 ); } );
  _params->addButton( "Prev", [this] { loadSample( _scene_index - 1 ); } );
  _params->addButton( "Restart Current", [this] { loadSample( _scene_index ); } );
#endif
  // Draw our app first, so samples show up over top.
  getWindow()->getSignalDraw().connect( 0, [this] {
    gl::clear( Color::black() );
#ifndef CINDER_COCOA_TOUCH
    _params->draw();
#endif
  } );

  loadSample( 0 );
  _timer.start();
}

void SamplesApp::loadSample( int index )
{
  bool do_animate = (index != _scene_index);
  const int start_x = (index < _scene_index) ? - getWindowWidth() : getWindowWidth();
  const int vanish_x = - start_x;

  float cooldown = ( _current_scene && _current_scene->timeline().empty() ) ? 0 : 0.33f;

  if( index < 0 ) { index = SampleList.size() - 1; }
  index %= SampleList.size();

  _scene_index = index;
  _scene_name = SampleNames[_scene_index];

  console() << "Loading Sample: " << _scene_name << endl;

  if( _current_scene && do_animate ) {
    _previous_scene = _current_scene;
    // animate off
    _timeline.apply( _previous_scene->getOffsetOutput() ).hold( cooldown ).then<RampTo>( vec2( vanish_x, 0.0f ), 0.4f, EaseInQuad() )
    .finishFn( [this] ( Motion<vec2> &m ) {
      _previous_scene.reset(); // get rid of previous scene
    } );
    _timeline.apply( _previous_scene->getAnimationSpeedOutput() ).then<RampTo>( 0, 0.4f );
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
    .finishFn( [this] ( Motion<vec2> &m ) {
      _current_scene->resume();
    } );

  }

  // If there was a previous cue lined up, cancel it.
  if( _cue_control ) {
    _cue_control->cancel();
  }

  // Load Next Sample Automatically.
  _cue_control = _timeline.cue( [this] {
    loadSample( _scene_index + 1 );
  }, 15.0f ).getControl();
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

CINDER_APP_NATIVE( SamplesApp, RendererGl )
