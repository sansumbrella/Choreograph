#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Choreograph.hpp"
#include "cinder/Easing.h"
#include "cinder/Rand.h"
#include "cinder/Timeline.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace choreograph;

class ChoreographDevApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
  void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
private:
  float                 _ball_y;
  ci::vec2              _ball_2 = ci::vec2( 400.0f, 400.0f );
  float                 _ball_radius = 50.0f;
  co::Timeline          _timeline;

  co::Output<vec2>      _mouse_queue;
  co::Output<vec2>      _mouse_move;
  co::Output<vec2>      _arc;
  co::Output<quat>      _orientation;

  vector<co::Output<vec2>>  _collection;
};

void ChoreographDevApp::setup()
{
  _timeline.move( &_ball_y )
  .startFn( [] (Motion<float> &c) { cout << "Start red" << endl; } )
  .getSequence().set( 5.0f ).hold( 0.5f ).rampTo( 500.0f, 1.0f, EaseInOutQuad() ).hold( 500.0f, 0.33f ).rampTo( 700.0f, 1.0f ).hold( 20.0f, 1.0f ).set( 400.0f );

  _timeline.move( &_ball_2 )
  .startFn( [] (Motion<vec2> &c) { cout << "Start blue" << endl; } )
  .finishFn( [] (Motion<vec2> &c) { c.playbackSpeed( c.getPlaybackSpeed() * -1.0f ); } )
  .continuous( true )
  .updateFn( [&] (const vec2 &v) {
    vec2 size = app::getWindowSize();
    float shortest = std::min( v.x, size.x - v.x );
    shortest = std::min( shortest, size.y - v.y );
    shortest = std::min( shortest, v.y );
    _ball_radius = shortest;
  } )
  .getSequence().rampTo( vec2( app::getWindowSize() ) / 2.0f, 2.0f ).rampTo( vec2( app::getWindowSize() ), 2.0f ).rampTo( vec2( app::getWindowWidth() / 2.0f, 10.0f ), 3.0f ).rampTo( vec2( app::getWindowSize() ) / 2.0f, 0.5f );

  _timeline.move<vec2, Phrase2v>( &_arc ).getSequence()
    .then( vec2( getWindowSize() ), 4.0f, EaseNone(), EaseInOutQuint() )
    .then( vec2( 0, getWindowHeight() / 2.0f ), 2.0f, EaseNone(), EaseInOutAtan() );
}

void ChoreographDevApp::mouseDown( MouseEvent event )
{
  _timeline.move( &_mouse_move ).getSequence().wait( 0.1f ).rampTo( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );
  _timeline.queue( &_mouse_queue ).wait( 0.1f ).rampTo( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );

  _timeline.queue( &_orientation ).rampTo( glm::angleAxis( (float)(randFloat() * M_PI * 2), randVec3f() ), 1.0f, EaseInOutCubic() );
}

void ChoreographDevApp::keyDown( KeyEvent event )
{
  _timeline.move<vec2, Phrase2v>( &_arc ).getSequence()
    .then( vec2( getWindowSize() ), 3.0f, EaseNone(), EaseInOutQuint() )
    .then( vec2( 0, getWindowHeight() / 2.0f ), 2.0f, EaseNone(), EaseInOutAtan() );
}

void ChoreographDevApp::update()
{
    _timeline.step( 1.0f / 60.0f );
}

void ChoreographDevApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

  gl::ScopedMatrices matrices;
  gl::setMatricesWindowPersp( getWindowSize(), 60.0f, 1.0f, 1000.0f );

  gl::disableDepthRead();
  gl::disableDepthWrite();

  gl::clear( Color( 0, 0, 0 ) );
  gl::color( Color( 1.0f, 0.0f, 0.0f ) );
  gl::drawSolidCircle( vec2( 200.0f, _ball_y ), 120.0f );
  gl::color( 0.0f, 0.0f, 1.0f );
  gl::drawSolidCircle( _ball_2, _ball_radius );

  gl::color( 0.0f, 1.0f, 0.0f );
  gl::drawSolidCircle( _mouse_queue, 40.0f );

  gl::color( Color( "steelblue" ) );
  gl::drawSolidCircle( _mouse_move, 30.0f );

  gl::color( Color( "magenta" ) );
  gl::drawSolidCircle( _arc, 30.0f );

  gl::enableDepthRead();
  gl::enableDepthWrite();
  gl::translate( getWindowCenter() );
  gl::rotate( _orientation );
  const int n = 3;
  for( int i = 0; i < n; ++i ) {
    vec3 pos = glm::mix( vec3( -100.0f, 0.0f, 0.0f ), vec3( 100.0f, 0.0f, 0.0f ), i / (n - 1.0f) );
    float size = mix( 20.0f, 60.0f, i / (n - 1.0f) );
    gl::drawColorCube( pos, vec3( size ) );
  }
}

CINDER_APP_NATIVE( ChoreographDevApp, RendererGl )
