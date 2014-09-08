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
  co::Output<float>     _copied;

  vector<co::Output<vec2>>  _collection;

  shared_ptr<co::Timeline>    CO_TIMELINE = make_shared<co::Timeline>();
  ci::TimelineRef             CI_TIMELINE = ci::Timeline::create();
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

  for( int j = 0; j < 10; ++j )
  {
    co::Timeline test_timeline;
    const int tween_count = 5000;
    const float dt = 1.0f / 60.0f;
    vector<co::Output<vec2>> targets( tween_count, vec2( 0 ) );
    ci::Timer perf( true );
    for( int i = 0; i < tween_count; ++i )
    {
      test_timeline.move( &targets[i] ).getSequence().hold( 1.0f ).rampTo( vec2( i * 5, i * 20 ), 2.0f );

      for( float t = 0.0f; t <= 3.0f; t += dt ) {
        test_timeline.step( dt );
      }
    }
    perf.stop();
    app::console() << "Virtual Phrases: " << perf.getSeconds() * 1000 << "ms" << endl;
  }

  _timeline.move( &_arc ).getSequence()
    .then<co::Phrase2<vec2>>( vec2( getWindowSize() ), 4.0f, EaseNone(), EaseInOutQuint() )
    // I want to be able to use the syntax on the next line
//    .then<co::Phrase2>( vec2( 50.0f, 50.0f ), 1.0f, EaseNone(), EaseInOutBounce() )
    .then( make_shared<Phrase2<vec2>>( vec2( 0, getWindowHeight() / 2.0f ), 2.0f, EaseNone(), EaseInOutAtan() ) );
}

void ChoreographDevApp::mouseDown( MouseEvent event )
{
  _timeline.move( &_mouse_move ).getSequence().wait( 0.1f ).rampTo( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );
  _timeline.queue( &_mouse_queue ).wait( 0.1f ).rampTo( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );

  if( false )
  {
    // Create a locally-scoped variable to test the MotionBase/OutputBase removal
    co::Output<float> temp = 5.0f;
    temp = 10.0f;
    _timeline.move( &temp ).getSequence().wait( 10.0f ).rampTo( 500.0f, 1.0f );
  }

  {
    // Copy a local output to a member output
    co::Output<float> temp = randFloat( getWindowWidth() );
    temp += 10.0f;
//    temp = randFloat( getWindowWidth() );
    _timeline.move( &temp ).getSequence().rampTo( randFloat( getWindowWidth() ), 1.25f, EaseInOutBack() );
    // we allow r-value copying, but don't allow standard copy-assignment because it might break your expectations as it modifies the rhs.
    _copied = std::move( temp );
  }

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

  gl::color( Color( "tomato" ) );
  gl::drawSolidCircle( vec2( _copied(), 20.0f ), 20.0f );

  gl::color( Color( "magenta" ) );
  gl::drawSolidCircle( _arc, 30.0f );
}

CINDER_APP_NATIVE( ChoreographDevApp, RendererGl )
