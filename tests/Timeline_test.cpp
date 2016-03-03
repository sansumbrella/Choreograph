//
//  Timeline_test.cpp
//
//  Created by Soso Limited on 10/26/15.
//
//

#include "catch.hpp"
#include "choreograph/Choreograph.h"

using namespace choreograph;
using namespace std;

TEST_CASE( "Timeline" )
{
  Timeline      timeline;
  Output<float> target = 0.0f;

  auto sequence = Sequence<float>( 0.0f )
    .then<RampTo>( 1.0f, 1.0f )
    .then<RampTo>( 10.0f, 1.0f )
    .then<RampTo>( 100.0f, 1.0f );

  SECTION( "Convenience methods create equivalent timeline to explicit methods" )
  {
    Output<float> a, b;
    timeline.apply(&a)
      .set(0.0f)
      .rampTo(5.0f, 1.0, EaseOutQuad());

    timeline.apply(&b)
      .set(0.0f)
      .then<RampTo>(5.0f, 1.0, EaseOutQuad());

    for (auto t = 0.0; t < 1.0; t += 0.2) {
      timeline.jumpTo(t);
      REQUIRE(a == b);
    }
  }

  SECTION( "Output<T> pointers can be controlled via Timeline." )
  {
    Output<float> target = 0.0f;

    // Construct Motion directly
    Motion<float> motion( &target, sequence );

    motion.jumpTo( 1.0f );
    REQUIRE( target == 1.0f );
    motion.jumpTo( 0.5f );
    REQUIRE( target == 0.5f );

    // Use timeline to create Motion.
    timeline.apply( &target, sequence );
    // Motions on Output types will be disconnected when another is created.
    REQUIRE( motion.cancelled() == true );

    timeline.jumpTo( 2.0f );
    REQUIRE( target == 10.0f );
  }

  SECTION( "Raw pointers can be controlled via Timeline." )
  {
    float target = 0.0f;
    // Construct Motion directly
    Motion<float> motion( &target, sequence );

    motion.jumpTo( 1.0f );
    REQUIRE( target == 1.0f );
    motion.jumpTo( 0.5f );
    REQUIRE( target == 0.5f );

    // Use timeline to create Motion.
    timeline.applyRaw( &target, sequence );
    // Known issue with raw pointers, no management will have happened.
    REQUIRE( motion.cancelled() == false );

    timeline.jumpTo( 2.0f );
    REQUIRE( target == 10.0f );
  }

  SECTION( "Timeline duration is a function of all motions." )
  {
    Output<float> other = 0.0f;
    auto options = timeline.apply( &other, sequence );

    SECTION( "Adjusting motion start time affects timeline duration." )
    {
      options.setStartTime( 1.0f );
      REQUIRE( timeline.timeUntilFinish() == 4.0f );
    }

    SECTION( "Adjusting motion playback speed affects timeline duration." )
    {
      options.playbackSpeed( 0.5f );
      REQUIRE( timeline.timeUntilFinish() == 6.0f );
    }
  }
} // Timeline

//==========================================
// Motion Callbacks on the Timeline
//==========================================

TEST_CASE( "Callbacks" )
{
  Timeline      timeline;
  Output<float> target = 0.0f;

  auto sequence = Sequence<float>( 0.0f )
    .then<RampTo>( 1.0f, 1.0f )
    .then<RampTo>( 10.0f, 1.0f )
    .then<RampTo>( 100.0f, 1.0f );

  auto options = timeline.apply( &target, sequence );

  SECTION( "Functions can be cued by motion events: start, update, and finish." )
  {
    bool          startCalled = false;
    bool          endCalled = false;
    int           updateCount = 0;
    float         updateTarget = 0;

    options.startFn( [&startCalled] () { startCalled = true; } )
      .updateFn( [&updateTarget, &target, &updateCount] () { updateTarget = target() / 2.0f; updateCount++; } )
      .finishFn( [&endCalled] () { endCalled = true; } );

    const float step = timeline.timeUntilFinish() / 10.0;
    timeline.step( step );
    REQUIRE( startCalled );
    REQUIRE( updateCount == 1 );
    REQUIRE( updateTarget == (target / 2.0) );

    for( int i = 0; i < 9; i += 1 ) {
      REQUIRE_FALSE( endCalled );
      timeline.step( step );
    }

    REQUIRE( endCalled );
    REQUIRE( updateCount == 10 );
  }

  SECTION( "Functions can be cued by sequence inflection points." )
  {
    int c1 = 0;
    int c2 = 0;
    int trigger_count = 0;

    timeline.apply( &target )
      .hold( 0.5f )
      // inflects around 0.5
      .onInflection( [&c1] () { c1 += 1; } )
      .then<RampTo>( 3.0f, 1.0f )
      // inflects around 1.5
      .onInflection( [&c2] () {
        c2 += 1;
      } )
      .then<RampTo>( 2.0f, 1.0f );

    timeline.append( &target )
      .onInflection( [&trigger_count] () { trigger_count += 1; } )
      .hold( 0.001 )
      .onInflection( [&trigger_count] () { trigger_count += 1; } )
      .hold( 1.0 );

    timeline.step( 0.49f );
    timeline.step( 0.02f );
    REQUIRE( c1 == 1 );
    REQUIRE( c2 == 0 );

    timeline.jumpTo( 1.51f );
    REQUIRE( c2 == 1 );
    REQUIRE( c1 == 1 );
    timeline.jumpTo( 1.49f );
    REQUIRE( c2 == 2 );
    REQUIRE( c1 == 1 );

    timeline.step( 2.0f );
    REQUIRE( trigger_count == 2 );
  }

  SECTION( "It is safe to add and cancel motions from Cues and Motion callbacks." )
  {
    Output<float> t2 = 1.0f;

    SECTION( "Add Motion from Motion callback." )
    {
      options.startFn( [&] () {
        timeline.apply( &t2, sequence );
      } );

      REQUIRE( timeline.size() == 1 );
      timeline.step( 0.1f );

      REQUIRE( timeline.size() == 2 );
    }

    SECTION( "Cancel Motion from Motion callback." )
    {
      // Play t2 motion twice as fast as previous, disconnect target on finish.
      timeline.apply( &t2, sequence )
        .playbackSpeed( 2.0f )
        .finishFn( [&target] () {
          target.disconnect();
      } );

      REQUIRE( timeline.size() == 2 );
      timeline.step( 1.5f );
      float v1 = target();
      float v2 = t2();

      REQUIRE( v1 == 5.5f );
      REQUIRE( v2 == 100.0f );
      REQUIRE( timeline.empty() );
    }

    SECTION( "Change Motion from Cue." )
    {
      timeline.cue( [&] {
        timeline.append( &target )
          .then( sequence );
      }, 0.5f );

      REQUIRE( timeline.timeUntilFinish() == 3.0f );
      timeline.step( 0.5f );

      REQUIRE( timeline.timeUntilFinish() == 5.5f );
    }
  }

  SECTION( "It is safe to destroy a Timeline from the Timeline finish fn." )
  {
    auto self_destructing_timeline = detail::make_unique<Timeline>();
    self_destructing_timeline->apply( &target, sequence );
    // Note, cleared has different semantics from finished.
    // TODO: add a test for the finish fn (or remove that fn).
    self_destructing_timeline->setClearedFn( [&self_destructing_timeline] {
      self_destructing_timeline.reset();
    } );

    REQUIRE( self_destructing_timeline );

    // Stepping forward gets to the end and triggers our finish fn.
    // Note that our application does not crash here (that's the real test).
    self_destructing_timeline->jumpTo( sequence.getDuration() );

    REQUIRE_FALSE( self_destructing_timeline );
  }
}
