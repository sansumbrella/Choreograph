#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Choreograph.hpp"
#include "specialization/CinderSpecialization.h"
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

  _timeline.move( &_arc ).getSequence()
    .then<co::Phrase2<vec2>>( vec2( getWindowSize() ), 4.0f, EaseNone(), EaseInOutQuint() )
    // I want to be able to use the syntax on the next line
//    .then<co::Phrase2>( vec2( 50.0f, 50.0f ), 1.0f, EaseNone(), EaseInOutBounce() )
    .then( make_shared<Phrase2v>( vec2( 0, getWindowHeight() / 2.0f ), 2.0f, EaseNone(), EaseInOutAtan() ) );
}

void ChoreographDevApp::mouseDown( MouseEvent event )
{
  _timeline.move( &_mouse_move ).getSequence().wait( 0.1f ).rampTo( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );
  _timeline.queue( &_mouse_queue ).wait( 0.1f ).rampTo( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );
}

void ChoreographDevApp::update()
{
    _timeline.step( 1.0f / 60.0f );
}

void ChoreographDevApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

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
}

CINDER_APP_NATIVE( ChoreographDevApp, RendererGl )
