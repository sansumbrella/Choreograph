
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "choreograph/Choreograph.h"

#include "cinder/Vector.h"
#include "cinder/Timeline.h"

#include <chrono>

using namespace std;
using cinder::Timeline;
using cinder::vec2;
using namespace choreograph;

using timer = chrono::high_resolution_clock;
// double millis
using d = chrono::duration<double, ratio<1, 1000>>;

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

void printTiming( const std::string &text, const timer::duration &duration )
{
  printTiming( text, chrono::duration_cast<d>( duration ).count(), "ms" );
}

void printHeading( const std::string &text )
{
  cout << endl << text << endl;
  cout << string( text.size(), '=' ) << endl;
}

TEST_CASE( "Sequence Manipulation Timing" )
{
  printHeading( "Sequence Creation and Slicing" );

  auto create_medium_begin = timer::now();
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

  auto create_medium_end = timer::now();
  printTiming( "Creating Medium Sequence", create_medium_end - create_medium_begin );

  auto create_auto_medium_begin = timer::now();
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

  auto create_auto_medium_end = timer::now();
  printTiming( "Creating Medium Sequence rhs Assignment", create_auto_medium_end - create_auto_medium_begin );

  auto create_huge_begin = timer::now();
  Sequence<float> huge_sequence( 0.0f );
  for( int i = 0; i < 1.0e6; i += 1 ) {
    huge_sequence.then<RampTo>( cos( i * 0.1 ) * (i - 1.0e3), 1.0f, EaseInOutCubic() );
  }
  auto create_huge_end = timer::now();
  printTiming( "Creating Huge Sequence", create_huge_end - create_huge_begin );

  auto slice_medium_begin = timer::now();
  auto sub_medium = medium_sequence.slice( 1.3f, 7.2f );
  auto slice_medium_end = timer::now();
  printTiming( "Slicing Medium Sequence", slice_medium_end - slice_medium_begin );

  auto slice_huge_begin = timer::now();
  // Performance is linear on the number of phrases before end time.
  auto sub_huge = huge_sequence.slice( 5.55f, 15000.025f );
  auto slice_huge_end = timer::now();
  printTiming( "Slicing Huge Sequence", slice_huge_end - slice_huge_begin );
}

TEST_CASE( "Choreograph Timeline Basic Performance" )
{

  ch::Timeline choreograph_timeline;
  choreograph_timeline.setDefaultRemoveOnFinish( false );
  float dt = 1.0f / 60.0f;
  const size_t count = 50e3;

  printHeading( "Basic Timing for " + to_string( count ) + " Motions" );
  vector<Output<vec2>> targets( count, vec2( 0 ) );
  auto create_in_place_begin = timer::now();
  for( auto &target : targets ) {
    choreograph_timeline.apply( &target ).then<RampTo>( vec2( 10.0f ), 1.0f ).startFn( [] (Motion<vec2> &) {} ).updateFn( [] ( const vec2 &value ) { value + vec2(1); } ).finishFn( [] (Motion<vec2> &) {} );
  }
  auto create_in_place_end = timer::now();
  printTiming( "Creating Motions with Callbacks in place", create_in_place_end - create_in_place_begin );

  auto step_created_in_place_begin = timer::now();
  for( int i = 0; i < 60; ++i ) {
    choreograph_timeline.step( dt );
  }
  auto step_created_in_place_end = timer::now();
  printTiming( "60 Motion Steps (1sec at 60Hz)", step_created_in_place_end - step_created_in_place_begin );

  auto disconnect_begin = timer::now();
  for( auto &target : targets ) {
    target.disconnect();
  }

  auto disconnect_end = timer::now();
  printTiming( "Disconnecting Motions", disconnect_end - disconnect_begin );

  auto step_disconnected_begin = timer::now();
  choreograph_timeline.step( dt );

  auto step_disconnected_end = timer::now();
  printTiming( "Step to remove disconnected Motions", step_disconnected_end - step_disconnected_begin );

  auto create_copy_begin = timer::now();
  Sequence<vec2> sequence( vec2( 1.0f ) );
  sequence.then<RampTo>( vec2( 5.0f ), 1.0f ).then<RampTo>( vec2( 10.0f, 6.0f ), 0.5f );
  for( auto &target : targets ) {
    choreograph_timeline.apply( &target, sequence ).startFn( [] (Motion<vec2> &) {} ).updateFn( [] ( const vec2 &value ) { value + vec2(1); } ).finishFn( [] (Motion<vec2> &) {} );
  }

  auto create_copy_end = timer::now();
  printTiming( "Creating Motions from existing Sequence", create_copy_end - create_copy_begin );


  auto step_copy_created_begin = timer::now();
  for( int i = 0; i < 60; ++i ) {
    choreograph_timeline.step( dt );
  }
  auto step_copy_created_end = timer::now();
  printTiming( "60 Motion Steps (1sec at 60Hz)", step_copy_created_end - step_copy_created_begin );

}

TEST_CASE( "Comparative Performance with cinder::Timeline" )
{
  ch::Timeline    choreograph_timeline;
  ci::TimelineRef cinder_timeline = ci::Timeline::create();

  const int               tween_count = 5000;
  vector<Output<vec2>>    targets( tween_count, vec2( 0 ) );
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
      auto ch_create_begin = timer::now();
      for( int i = 0; i < tween_count; ++i ) {
        choreograph_timeline.apply( &targets[i] )
          .then<Hold>( vec2( 0 ), 1.0f )
          .then<RampTo>( vec2( i * 5, i * 20 ), 2.0f );
      }
      auto ch_create_end = timer::now();

      // Create Cinder Tweens
      auto ci_create_begin = timer::now();
      for( int i = 0; i < tween_count; ++i ) {
        cinder_timeline->apply( &cinder_targets[i], vec2( i * 5, i * 20 ), 2.0f )
          .delay( 1.0f );
      }
      auto ci_create_end = timer::now();

      auto ch_step_begin = timer::now();
      // Step through Choreograph Motions
      for( float t = 0.0f; t <= total_time; t += dt ) {
        choreograph_timeline.step( dt );
      }
      auto ch_step_end = timer::now();

      auto ci_step_begin = timer::now();
      // Step through Cinder Tweens
      for( float t = 0.0f; t <= total_time; t += dt ) {
        cinder_timeline->step( dt );
      }
      auto ci_step_end = timer::now();

      ci_create_avg += chrono::duration_cast<d>( ci_create_end - ci_create_begin ).count();
      ci_step_avg += chrono::duration_cast<d>( ci_step_end - ci_step_begin ).count();

      ch_create_avg += chrono::duration_cast<d>( ch_create_end - ch_create_begin ).count();
      ch_step_avg += chrono::duration_cast<d>( ch_step_end - ch_step_begin ).count();
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
      auto ch_create_begin = timer::now();
      for( int i = 0; i < tween_count; ++i ) {
        choreograph_timeline.append( &target ).then<Hold>( vec2( 0 ), 1.0f ).then<RampTo>( vec2( i * 5, i * 20 ), 2.0f );
      }
      auto ch_create_end = timer::now();

      // Create Cinder Tweens
      auto ci_create_begin = timer::now();
      for( int i = 0; i < tween_count; ++i ) {
        cinder_timeline->appendTo( &cinder_target, vec2( i * 5, i * 20 ), 2.0f ).delay( 1.0f );
      }
      auto ci_create_end = timer::now();

      // Step through Choreograph Motions
      auto ch_step_begin = timer::now();
      for( float t = 0.0f; t <= total_time; t += dt ) {
        choreograph_timeline.step( dt );
      }
      auto ch_step_end = timer::now();

      // Step through Cinder Tweens
      auto ci_step_begin = timer::now();
      for( float t = 0.0f; t <= total_time; t += dt ) {
        cinder_timeline->step( dt );
      }
      auto ci_step_end = timer::now();

      ci_create_avg += chrono::duration_cast<d>( ci_create_end - ci_create_begin ).count();
      ci_step_avg += chrono::duration_cast<d>( ci_step_end - ci_step_begin ).count();

      ch_create_avg += chrono::duration_cast<d>( ch_create_end - ch_create_begin ).count();
      ch_step_avg += chrono::duration_cast<d>( ch_step_end - ch_step_begin ).count();
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
