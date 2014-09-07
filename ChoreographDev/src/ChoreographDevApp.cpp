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

  void performanceCheckCinder();
  void performanceCheckChoreograph();
private:
  float                 _ball_y;
  ci::vec2              _ball_2 = ci::vec2( 400.0f, 400.0f );
  float                 _ball_radius = 50.0f;
  co::Timeline          _anim;

  co::Output<vec2>      _mouse_follower;
  co::Output<float>     _copied;

  vector<co::Output<vec2>>  _collection;

  shared_ptr<co::Timeline>    CO_TIMELINE = make_shared<co::Timeline>();
  ci::TimelineRef             CI_TIMELINE = ci::Timeline::create();
};

const int TEST_ANIMS = 10000;
const float TEST_DURATION = 0.2f;
const float TEST_DT = 1.0f / 60.0f;

void ChoreographDevApp::setup()
{
  for( int i = 0; i < 50; ++i )
  {
    CI_TIMELINE->clear();
    CI_TIMELINE->stepTo( 0.0f );
    CO_TIMELINE->clear();

    vector<ci::Anim<vec2>> cinder_anims( TEST_ANIMS, vec2( 0 ) );
    vector<co::Output<vec2>> choreograph_outputs( TEST_ANIMS, vec2( 0 ) );

    for( int i = 0; i < TEST_ANIMS; ++i )
    {
      CI_TIMELINE->apply( &cinder_anims[i], vec2( i, i * 2 ), TEST_DURATION );
      CO_TIMELINE->move( &choreograph_outputs[i] ).getSequence().rampTo( vec2( i, i * 2 ), TEST_DURATION );
    }


    Timer ci_timer( true );
    performanceCheckCinder();
    ci_timer.stop();

    Timer co_timer( true );
    performanceCheckChoreograph();
    co_timer.stop();


    console() << "Cinder: " << ci_timer.getSeconds() << ", Choreograph: " << co_timer.getSeconds() << endl;
    console() << "Time ratio (Cinder/Choreograph): " << ci_timer.getSeconds() / co_timer.getSeconds() << endl;
  }

  _anim.move( &_ball_y )
  .startFn( [] (Motion<float> &c) { cout << "Start red" << endl; } )
  .getSequence().set( 5.0f ).hold( 0.5f ).rampTo( 500.0f, 1.0f, EaseInOutQuad() ).hold( 500.0f, 0.33f ).rampTo( 700.0f, 1.0f ).hold( 20.0f, 1.0f ).set( 400.0f );

  _anim.move( &_ball_2 )
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

}

void ChoreographDevApp::performanceCheckCinder()
{
  for( float t = 0.0f; t < TEST_ANIMS * TEST_DURATION; t += TEST_DT )
  {
    CI_TIMELINE->step( TEST_DT );
  }
}

void ChoreographDevApp::performanceCheckChoreograph()
{
  for( float t = 0.0f; t < TEST_ANIMS * TEST_DURATION; t += TEST_DT )
  {
    CO_TIMELINE->step( TEST_DT );
  }
}

void ChoreographDevApp::mouseDown( MouseEvent event )
{
//  _anim.move( &_mouse_follower ).finishFn( [] { app::console() << "Mouse Anim Finished" << endl; } ).getSequence().wait( 0.1f ).rampTo( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );
  _anim.queue( &_mouse_follower ).wait( 0.1f ).rampTo( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );

  if( false )
  {
    // Create a locally-scoped variable to test the MotionBase/OutputBase removal
    co::Output<float> temp = 5.0f;
    temp = 10.0f;
    _anim.move( &temp ).getSequence().wait( 10.0f ).rampTo( 500.0f, 1.0f );
  }

  {
    // Copy a local output to a member output
    co::Output<float> temp = randFloat( getWindowWidth() );
    temp += 10.0f;
//    temp = randFloat( getWindowWidth() );
    _anim.move( &temp ).getSequence().rampTo( randFloat( getWindowWidth() ), 1.25f, EaseInOutBack() );
    // we allow r-value copying, but don't allow standard copy-assignment because it might break your expectations as it modifies the rhs.
    _copied = std::move( temp );
  }

}

void ChoreographDevApp::update()
{
    _anim.step( 1.0f / 60.0f );
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
  gl::drawSolidCircle( _mouse_follower, 40.0f );

  gl::drawSolidCircle( vec2( _copied(), 20.0f ), 20.0f );
}

CINDER_APP_NATIVE( ChoreographDevApp, RendererGl )
