
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "choreograph/Choreograph.h"

#include "cinder/Vector.h"
#include "cinder/Timeline.h"
#include "cinder/Timer.h"

#include <chrono>

using namespace std;
using cinder::Timeline;
using cinder::vec2;
using cinder::Timer;
using namespace choreograph;

void printTiming( const std::string &text, double milliseconds, const std::string &suffix = "ms" )
{
  string message = "[" + text + "] ";
  if( message.size() < 50 ) {
    message = message + string( 50 - message.size(), '.' );
  }

  auto number = to_string( milliseconds );
  auto pos = number.find( "." );
  if( pos != string::npos && pos < 4 ) {
    number = string( 4 - pos, ' ' ) + number + suffix;
  }
  cout << message << number << endl;
}

void printHeading( const std::string &text )
{
  cout << endl << text << endl;
  cout << string( text.size(), '=' ) << endl;
}

TEST_CASE( "Sequence Manipulation Timing" )
{
  printHeading( "Sequence Creation and Slicing" );

  Timer create_medium( true );
  Sequence<float> medium_sequence( 0.0f );
  medium_sequence.then<RampTo>( 10.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 1.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 20.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 2.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 40.0f, 1.0f, EaseInOutQuad() )
    .then<Hold>( 1000.0f, 0.5f )
    .then<RampTo>( 4.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 50.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 5.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 60.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 6.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 70.0f, 1.0f, EaseInOutQuad() );

  create_medium.stop();
  printTiming( "Creating Medium Sequence", create_medium.getSeconds() * 1000 );

  Timer create_auto_medium( true );
  auto auto_medium_sequence = Sequence<float>( 0.0f )
    .then<RampTo>( 1.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 20.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 2.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 40.0f, 1.0f, EaseInOutQuad() )
    .then<Hold>( 1000.0f, 0.5f )
    .then<RampTo>( 4.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 50.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 5.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 60.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 6.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 70.0f, 1.0f, EaseInOutQuad() )
    .then<RampTo>( 10.0f, 1.0f, EaseInOutQuad() );

  create_auto_medium.stop();
  printTiming( "Creating Medium Sequence rhs Assignment", create_auto_medium.getSeconds() * 1000 );

  Timer create_huge( true );
  Sequence<float> huge_sequence( 0.0f );
  for( int i = 0; i < 1.0e6; i += 1 ) {
    huge_sequence.then<RampTo>( cos( i * 0.1 ) * (i - 1.0e3), 1.0f, EaseInOutCubic() );
  }
  create_huge.stop();
  printTiming( "Creating Huge Sequence", create_huge.getSeconds() * 1000 );

  Timer slice_medium( true );
  auto sub_medium = medium_sequence.slice( 1.3f, 7.2f );
  slice_medium.stop();
  printTiming( "Slicing Medium Sequence", slice_medium.getSeconds() * 1000 );

  Timer slice_huge( true );
  // Performance is linear on the number of phrases before end time.
  auto sub_huge = huge_sequence.slice( 5.55f, 15000.025f );
  slice_huge.stop();
  printTiming( "Slicing Huge Sequence", slice_huge.getSeconds() * 1000 );
}

TEST_CASE( "Choreograph Timeline Basic Performance" )
{

  ch::Timeline choreograph_timeline;
  choreograph_timeline.setDefaultRemoveOnFinish( false );
  float dt = 1.0f / 60.0f;
  const size_t count = 50e3;

  printHeading( "Basic Timing for " + to_string( count ) + " Motions" );
  vector<Output<vec2>> targets;
  while( targets.size() < count ) {
    targets.emplace_back( vec2( 0 ) );
  }
  Timer create_in_place( true );
  for( auto &target : targets ) {
    choreograph_timeline.apply( &target ).then<RampTo>( vec2( 10.0f ), 1.0f ).startFn( [] {} ).updateFn( [&m = *target.inputPtr()] { m.getCurrentValue() + vec2(1); } ).finishFn( [] {} );
  }
  create_in_place.stop();
  printTiming( "Creating Motions with Callbacks in place", create_in_place.getSeconds() * 1000 );

  Timer step_created_in_place( true );
  for( int i = 0; i < 60; ++i ) {
    choreograph_timeline.step( dt );
  }
  step_created_in_place.stop();
  printTiming( "60 Motion Steps (1sec at 60Hz)", step_created_in_place.getSeconds() * 1000 );

  Timer disconnect( true );
  for( auto &target : targets ) {
    target.disconnect();
  }

  disconnect.stop();
  printTiming( "Disconnecting Motions", disconnect.getSeconds() * 1000 );

  Timer step_disconnected( true );
  choreograph_timeline.step( dt );

  step_disconnected.stop();
  printTiming( "Step to remove disconnected Motions", step_disconnected.getSeconds() * 1000 );

  Timer create_copy( true );
  Sequence<vec2> sequence( vec2( 1.0f ) );
  sequence.then<RampTo>( vec2( 5.0f ), 1.0f ).then<RampTo>( vec2( 10.0f, 6.0f ), 0.5f );
  for( auto &target : targets ) {
    choreograph_timeline.apply( &target, sequence ).startFn( [] {} ).updateFn( [&m = *target.inputPtr()] { m.getCurrentValue() + vec2(1); } ).finishFn( [] {} );
  }

  create_copy.stop();
  printTiming( "Creating Motions from existing Sequence", create_copy.getSeconds() * 1000 );


  Timer step_copy_created( true );
  for( int i = 0; i < 60; ++i ) {
    choreograph_timeline.step( dt );
  }
  step_copy_created.stop();
  printTiming( "60 Motion Steps (1sec at 60Hz)", step_copy_created.getSeconds() * 1000 );

}

TEST_CASE( "Comparative Performance with cinder::Timeline" )
{
  ch::Timeline    choreograph_timeline;
  ci::TimelineRef cinder_timeline = ci::Timeline::create();

  const int               tween_count = 5000;
  vector<Output<vec2>>    targets;
  while ( targets.size() < tween_count ) {
    targets.emplace_back( vec2( 0 ) );
  }
  vector<ci::Anim<vec2>>  cinder_targets( tween_count, vec2( 0 ) );

  double ci_step_avg    = 0;
  double ch_step_avg    = 0;
  double ci_create_avg  = 0;
  double ch_create_avg  = 0;
  double iterations     = 4;

  SECTION( "Tween many targets" )
  {
    printHeading( "Comparative Performance: many targets, short animations." );
    const float total_time = 3.0f;
    const float dt = total_time / 1000;

    for( int step = 0; step < iterations; step += 1 )
    {
      // Create Choreograph Motions
      Timer ch_create( true );
      for( int i = 0; i < tween_count; ++i ) {
        choreograph_timeline.apply( &targets[i] )
          .then<Hold>( vec2( 0 ), 1.0f )
          .then<RampTo>( vec2( i * 5, i * 20 ), 2.0f );
      }
      ch_create.stop();

      // Create Cinder Tweens
      Timer ci_create( true );
      for( int i = 0; i < tween_count; ++i ) {
        cinder_timeline->apply( &cinder_targets[i], vec2( i * 5, i * 20 ), 2.0f )
          .delay( 1.0f );
      }
      ci_create.stop();

      Timer ch_step( true );
      // Step through Choreograph Motions
      for( float t = 0.0f; t <= total_time; t += dt ) {
        choreograph_timeline.step( dt );
      }
      ch_step.stop();

      Timer ci_step( true );
      // Step through Cinder Tweens
      for( float t = 0.0f; t <= total_time; t += dt ) {
        cinder_timeline->step( dt );
      }
      ci_step.stop();

      ci_create_avg += ci_create.getSeconds() * 1000;
      ci_step_avg += ci_step.getSeconds() * 1000;

      ch_create_avg += ch_create.getSeconds() * 1000;
      ch_step_avg += ch_step.getSeconds() * 1000;
    }

    ci_create_avg /= iterations;
    ci_step_avg /= iterations;
    ch_create_avg /= iterations;
    ch_step_avg /= iterations;

    printTiming( "Choreograph Create Average", ch_create_avg );
    printTiming( "Cinder Create Average", ci_create_avg );
    printTiming( "Choreograph Step Average", ch_step_avg );
    printTiming( "Cinder Step Average", ci_step_avg );
    printTiming( "Create Performance (Choreograph / Cinder)", (ch_create_avg / ci_create_avg), "" );
    printTiming( "Step Performance (Choreograph / Cinder)", (ch_step_avg / ci_step_avg), "" );
  }

  SECTION( "Tween one target many times" )
  {
    printHeading( "Comparative Performance: single target, long animation." );
    const float total_time = tween_count * 3.0f;
    const float dt = total_time / 1000;

    for( int j = 0; j < iterations; j += 1 )
    {
      Output<vec2>    target( vec2( 0 ) );
      ci::Anim<vec2>  cinder_target( vec2( 0 ) );

      // Create Choreograph Motions
      Timer ch_create( true );
      for( int i = 0; i < tween_count; ++i ) {
        choreograph_timeline.append( &target ).then<Hold>( vec2( 0 ), 1.0f ).then<RampTo>( vec2( i * 5, i * 20 ), 2.0f );
      }
      ch_create.stop();

      // Create Cinder Tweens
      Timer ci_create( true );
      for( int i = 0; i < tween_count; ++i ) {
        cinder_timeline->appendTo( &cinder_target, vec2( i * 5, i * 20 ), 2.0f ).delay( 1.0f );
      }
      ci_create.stop();

      // Step through Choreograph Motions
      Timer ch_step( true );
      for( float t = 0.0f; t <= total_time; t += dt ) {
        choreograph_timeline.step( dt );
      }
      ch_step.stop();

      // Step through Cinder Tweens
      Timer ci_step( true );
      for( float t = 0.0f; t <= total_time; t += dt ) {
        cinder_timeline->step( dt );
      }
      ci_step.stop();

      ci_create_avg += ci_create.getSeconds() * 1000;
      ci_step_avg += ci_step.getSeconds() * 1000;

      ch_create_avg += ch_create.getSeconds() * 1000;
      ch_step_avg += ch_step.getSeconds() * 1000;
    }

    ci_create_avg /= iterations;
    ci_step_avg /= iterations;
    ch_create_avg /= iterations;
    ch_step_avg /= iterations;

    printTiming( "Choreograph Create Average", ch_create_avg );
    printTiming( "Cinder Create Average", ci_create_avg );
    printTiming( "Choreograph Step Average", ch_step_avg );
    printTiming( "Cinder Step Average", ci_step_avg );
    printTiming( "Create Performance (Choreograph / Cinder)", (ch_create_avg / ci_create_avg), "" );
    printTiming( "Step Performance (Choreograph / Cinder)", (ch_step_avg / ci_step_avg), "" );
  }
}
