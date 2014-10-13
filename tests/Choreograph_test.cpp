
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "choreograph/Choreograph.h"
#include <array>

// If true, will test vec2 and vec3 animation.
#define INCLUDE_CINDER_HEADERS 1

using namespace std;
using namespace choreograph;

TEST_CASE( "Sequence Interpolation", "[sequence]" )
{
  // Since the precision of floats decreases as they get larger,
  // we need an epsilon larger than the delta from 1.0 to the next float.
  const float epsilon = std::numeric_limits<float>::epsilon() * 20;

  Sequence<float> sequence( 0.0f );
  // 4 second sequence.
  sequence.set( 1.0f ).then<Hold>( 1.0f, 1.0f ).then<RampTo>( 2.0f, 1.0f ).then<RampTo>( 10.0f, 1.0f ).then<RampTo>( 2.0f, 1.0f );

  SECTION( "Sequence values within duration are correct." ) {
    REQUIRE( sequence.getValue( 0.5f ) == 1.0f );
    REQUIRE( sequence.getValue( 1.0f ) == 1.0f );
    REQUIRE( sequence.getValue( 1.5f ) == 1.5f );
  }

  SECTION( "Sequence values outside duration are correct." ) {
    REQUIRE( sequence.getValue( std::numeric_limits<float>::min() ) == 1.0f );
    REQUIRE( sequence.getValue( std::numeric_limits<float>::max() ) == 2.0f );
  }

  SECTION( "Looped sequence values are correct." ) {
    float offset = 2.015f;

    REQUIRE( (wrapTime( 10 * sequence.getDuration() + offset, sequence.getDuration() ) - offset) < epsilon );
    REQUIRE( (sequence.getValueWrapped( sequence.getDuration() + offset ) - sequence.getValue( offset ) ) < epsilon );
    REQUIRE( (sequence.getValueWrapped( (2 * sequence.getDuration()) + offset ) - sequence.getValue( offset ) ) < epsilon );
    REQUIRE( (sequence.getValueWrapped( (50 * sequence.getDuration()) + offset ) - sequence.getValue( offset ) ) < epsilon );
  }
}

TEST_CASE( "Time and Infinity" )
{
  const Time infinity = std::numeric_limits<Time>::infinity();

  Sequence<float> sequence( 0.0f );
  sequence.then<RampTo>( 1.0f, 1.0f, EaseInOutQuad() ).then<RampTo>( 2.0f, infinity );

  PhraseRef<float> ramp = make_shared<RampTo<float>>( 2.0f, 0.0f, 10.0f );
  auto looped = makeRepeat<float>( ramp, infinity );

  REQUIRE( looped->getValue( 1.0f ) == looped->getValue( 2001.0f ) );
  REQUIRE( sequence.getValue( 1.0f ) == 1.0f );
  REQUIRE( sequence.getValue( 2.0f ) == 1.0f );
  REQUIRE( sequence.getDuration() == infinity );
  REQUIRE( sequence.getValue( infinity ) == 2.0f );
  REQUIRE( (1000.0f / infinity) == 0.0f );
} // Time and Infinity

TEST_CASE( "Raw Pointers" )
{
  float target = 0.0f;
  Timeline timeline;

  SECTION( "Composing Sequences in Sequences" ) {
    auto sequence = createSequence( 1.0f );
    SequenceRef<float> continuation = createSequence( 5.0f );
    continuation->then<RampTo>( 10.0f, 1.0f ).then<Hold>( 3.0f, 1.0f );

    sequence->then<RampTo>( 5.0f, 0.5f ).then( *continuation ).then( continuation->asPhrase() );

    REQUIRE( sequence->getDuration() == 4.5f );

    Motion<float> motion( &target, sequence );
    motion.jumpTo( 1.0f );
    REQUIRE( target == 7.5f );

    motion.jumpTo( 1.5f );
    REQUIRE( target == 10.0f );

    timeline.applyRaw( &target, sequence );
    REQUIRE( motion.isInvalid() == false ); // Working with raw pointer, no management will have happened.

    timeline.jumpTo( 0.0f );
    REQUIRE( target == 1.0f );

    timeline.step( 1.0f );
    REQUIRE( target == 7.5f );

    timeline.step( 0.5f );
    REQUIRE( target == 10.0f );

    timeline.step( 0.5f );
    REQUIRE( target == 3.0f );
  }

}

TEST_CASE( "Sequence Composition", "[sequence]" )
{
  Output<float> target = 0.0f;
  Timeline timeline;

  SECTION( "Composing Sequences in Sequences" ) {
    auto sequence = createSequence( 1.0f );
    SequenceRef<float> continuation = createSequence( 5.0f );
    continuation->then<RampTo>( 10.0f, 1.0f ).then<Hold>( 3.0f, 1.0f );

    sequence->then<RampTo>( 5.0f, 0.5f ).then( *continuation ).then( continuation->asPhrase() );

    REQUIRE( sequence->getDuration() == 4.5f );

    Motion<float> motion( &target, sequence );
    motion.jumpTo( 1.0f );
    REQUIRE( target == 7.5f );

    motion.jumpTo( 1.5f );
    REQUIRE( target == 10.0f );

    timeline.apply( &target, sequence );
    REQUIRE( motion.isInvalid() == true ); // management will invalidate the motion (because it's been superseded).

    timeline.jumpTo( 0.0f );
    REQUIRE( target == 1.0f );

    timeline.step( 1.0f );
    REQUIRE( target == 7.5f );

    timeline.step( 0.5f );
    REQUIRE( target == 10.0f );

    timeline.step( 0.5f );
    REQUIRE( target == 3.0f );
  }
}

TEST_CASE( "Cues and Callbacks", "[motion]" )
{
  ch::Timeline  timeline;

  SECTION( "Timeline Callbacks" )
  {
    bool          updateCalled = false;
    bool          startCalled = false;
    bool          endCalled = false;
    Output<float> target = 0;
    int           updateCount = 0;
    float         updateTarget = 0;

    timeline.apply( &target ).startFn( [&startCalled] (Motion<float> &) { startCalled = true; } )
      .updateFn( [&updateCalled, &updateTarget, &updateCount] ( float value ) { updateTarget = value / 2.0f; updateCount++; updateCalled = true; } )
      .finishFn( [&endCalled] (Motion<float> &) { endCalled = true; } )
      .then<RampTo>( 10.0f, 1.0f );

    SECTION( "Callbacks from step" )
    {
      timeline.step( 0.1f );
      REQUIRE( startCalled == true );
      REQUIRE( updateCalled == true );
      REQUIRE( updateCount == 1 );
      REQUIRE( updateTarget == (target / 2.0f) );
      REQUIRE( target == 1.0f );

      for( int i = 0; i < 9; ++i ) {
        REQUIRE( endCalled == false );
        timeline.step( 0.1f );
      }

      REQUIRE( endCalled == true );
      REQUIRE( updateCount == 10 );
    }

    SECTION( "Callbacks from jumpTo" )
    {
      timeline.jumpTo( 0.1f );
      REQUIRE( startCalled == true );
      REQUIRE( updateCalled == true );
      REQUIRE( updateCount == 1 );
      REQUIRE( updateTarget == (target / 2.0f) );
      REQUIRE( target == 1.0f );
      REQUIRE( endCalled == false );

      timeline.jumpTo( 0.9f );
      REQUIRE( updateCount == 2 );
      REQUIRE( endCalled == false );

      timeline.jumpTo( 1.0f );
      REQUIRE( updateCount == 3 );
      REQUIRE( endCalled == true );
    }
  }

  SECTION( "Cues" )
  {
    vector<int> call_counts( 4, 0 );
    std::array<float, 4> delays = { 0.01f, 1.0f, 2.0f, 3.0f };
    for( size_t i = 0; i < call_counts.size(); ++i )
    {
      timeline.cue( [i, &call_counts] { call_counts[i] += 1; }, delays[i] );
    }

    timeline.step( 0.1f );
    REQUIRE( call_counts[0] == 1 );
    REQUIRE( call_counts[1] == 0 );
    REQUIRE( call_counts[2] == 0 );

    for( int i = 0; i < 9; ++i ) {
      timeline.step( 0.1f );
    }

    REQUIRE( call_counts[0] == 1 );
    REQUIRE( call_counts[1] == 1 );
    REQUIRE( call_counts[2] == 0 );

    for( int i = 0; i < 10; ++i ) {
      timeline.step( 0.1f );
    }

//    REQUIRE( timeline.size() == 1 );
    REQUIRE( call_counts[0] == 1 );
    REQUIRE( call_counts[1] == 1 );
    REQUIRE( call_counts[2] == 1 );
    REQUIRE( call_counts[3] == 0 );

    for( int i = 0; i < 11; ++i ) {
      timeline.step( 0.1f );
    }

//    REQUIRE( timeline.size() == 0 );
    REQUIRE( call_counts[0] == 1 );
    REQUIRE( call_counts[1] == 1 );
    REQUIRE( call_counts[2] == 1 );
    REQUIRE( call_counts[3] == 1 );
  }

  SECTION( "Cue Scoping and Cancellation" )
  {
    int call_count = 0;

    SECTION( "Plain Handle" )
    {
      {
        auto handle = timeline.cue( [&call_count] { call_count += 1; }, 1.0f ).getControl();
      }
      timeline.jumpTo( 1.0f );
      REQUIRE( call_count == 1 );
    }

    SECTION( "Plain Handle, cancelled" )
    {
      {
        auto handle = timeline.cue( [&call_count] { call_count += 1; }, 1.0f ).getControl();
        auto locked = handle.lock();
        if( locked ) {
          locked->cancel();
        }
      }
      timeline.jumpTo( 1.0f );
      REQUIRE( call_count == 0 );
    }

    SECTION( "Scoped Control, falls" )
    {
      {
        auto handle = timeline.cue( [&call_count] { call_count += 1; }, 1.0f ).getScopedControl();
      }
      timeline.jumpTo( 1.0f );
      REQUIRE( call_count == 0 );
    }

    SECTION( "Scoped Control, lives" )
    {
      ScopedCueRef cue;
      {
        cue = timeline.cue( [&call_count] { call_count += 1; }, 1.0f ).getScopedControl();
      }
      timeline.jumpTo( 1.0f );
      REQUIRE( call_count == 1 );
    }
  }

  SECTION( "Motion Manipulation from Callbacks" )
  {

  }
}

TEST_CASE( "Motion Speed and Reversal" )
{
  ch::Timeline timeline;
  auto sequence = createSequence( 0.0f );
  sequence->then<RampTo>( 10.0f, 1.0f ).then<RampTo>( -30.0f, 2.0f );

  SECTION( "Equivalence between motion time and sequence time" )
  {
    Output<float> target;
    timeline.setDefaultRemoveOnFinish( false );
    timeline.apply( &target, sequence );

    vector<Time> times = { 0.5, 0.2f, 1.0, 0.0, 2.0, 2.5, 3.0, 0.0, 0.3f, 0.5 };
    for( auto &t : times )
    {
      timeline.jumpTo( t );
      REQUIRE( target() == sequence->getValue( t ) );
    }
  }

  SECTION( "Walking Backwards" )
  {

  }

  SECTION( "Walking Slowly" )
  {

  }

  SECTION( "Walking In Both Directions" )
  {

  }

} // Motion Speed and Reversal

TEST_CASE( "Output Connections", "[output]" )
{

  ch::Timeline timeline;
  auto sequence = make_shared<Sequence<float>>( 0.0f );
  sequence->then<RampTo>( 10.0f, 2.0f );

  SECTION( "Output falling out of scope disconnects" ) {
    MotionRef<float> motion;

    { // create locally scoped output
      Output<float> temp;
      motion = make_shared<Motion<float>>( &temp, sequence );
      REQUIRE( ! motion->isInvalid() );
      REQUIRE( temp.isConnected() );
    }

    REQUIRE( motion->isInvalid() );
  }

  SECTION( "Motion falling out of scope disconnects" ) {
    Output<float> output;

    { // hook up a motion to our output
      Motion<float> temp( &output, sequence );

      REQUIRE( output.isConnected() == true );
      REQUIRE( temp.isInvalid() == false );
    }

    REQUIRE( output.isConnected() == false );
  }

  SECTION( "Timeline Removes Invalid Connections" ) {
    { // create locally scoped output
      Output<float> temp;
      timeline.apply( &temp ).then<RampTo>( 5.0f, 1.0f );

      REQUIRE( timeline.size() == 1 );
    }

    // Part of test is that nothing fails when stepping the timeline.
    timeline.step( 0.5f );
    REQUIRE( timeline.empty() == true );
  }

  SECTION( "Vector of outputs can be moved." ) {
    vector<Output<float>> outputs( 500, 0.0f );
    vector<Output<float>> copy;

    for( auto &output : outputs ) {
      timeline.apply( &output, sequence );
    }
    copy = std::move( outputs );

    timeline.step( 1.0f );
    bool all_five = true;
    for( auto &c : copy ) {
      if( c != 5.0f ) {
        all_five = false;
      }
    }
    REQUIRE( copy.front() == 5.0f );
    REQUIRE( all_five == true );
  }

  SECTION( "Move assignment brings motion along." ) {
    Output<float> base( 1.0f );
    Output<float> copy( 0.0f );

    Motion<float> motion( &base, sequence );
    copy = std::move( base );
    motion.jumpTo( 1.0f );

    REQUIRE( copy.value() == 5.0f );

    motion.jumpTo( 2.0f );
    REQUIRE( copy.value() == 10.0f );
  }

  SECTION( "Vector of outputs can be copied." ) {
    vector<Output<float>> outputs( 500, 0.0f );
    vector<Output<float>> copy;

    for( auto &output : outputs ) {
      timeline.apply( &output, sequence );
    }
    copy = outputs;

    timeline.step( 1.0f );
    bool all_five = true;
    for( auto &c : copy ) {
      if( c != 5.0f ) {
        all_five = false;
      }
    }
    REQUIRE( copy.front() == 5.0f );
    REQUIRE( all_five == true );
    REQUIRE( outputs.front() == 0.0f );
  }

  SECTION( "Copy assignment brings motion along." ) {
    Output<float> base( 1.0f );
    Output<float> copy( 0.0f );

    Motion<float> motion( &base, sequence );
    copy = base;
    motion.jumpTo( 1.0f );

    REQUIRE( base.value() == 1.0f );
    REQUIRE( copy.value() == 5.0f );

    motion.jumpTo( 2.0f );
    REQUIRE( copy.value() == 10.0f );
  }
} // Output Connections

#if INCLUDE_CINDER_HEADERS

#include "cinder/Vector.h"
#include <array>
using namespace cinder;

TEST_CASE( "Separate component interpolation", "[sequence]" )
{

  SECTION( "Compare 2-Component Values" ) {
    // Animate XY from and to same values with different ease curves.
    Sequence<vec2> sequence( vec2( 1 ) );
    sequence.then<RampTo2>( vec2( 10.0f ), 1.0f, EaseOutQuad(), EaseInQuad() );
    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).y );
    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).y );
    REQUIRE( sequence.getValue( 2.0f ).x == sequence.getValue( 2.0f ).y );  // past the end
    REQUIRE( sequence.getValue( 0.5f ).x != sequence.getValue( 0.5f ).y );
  }

  SECTION( "Compare 3-Component Values" ) {
    Sequence<vec3> sequence( vec3( 1 ) );
    sequence.then<RampTo3>( vec3( 10.0f ), 1.0f, EaseOutQuad(), EaseInQuad(), EaseInOutQuad() );

    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).y );
    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).z );

    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).y );
    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).z );

    REQUIRE( sequence.getValue( 0.5f ).x != sequence.getValue( 0.5f ).y );
    REQUIRE( sequence.getValue( 0.5f ).y != sequence.getValue( 0.5f ).z );
    REQUIRE( sequence.getValue( 0.5f ).x != sequence.getValue( 0.5f ).z );
  }

  SECTION( "Compare 4-Component Values" ) {
    Sequence<vec4> sequence( vec4( 1 ) );
    sequence.then<RampTo4>( vec4( 10.0f ), 1.0f, EaseOutQuad(), EaseInAtan(), EaseInOutQuad(), EaseInCubic() );

    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).y );
    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).z );

    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).y );
    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).z );
    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).w );

    REQUIRE( sequence.getValue( 0.5f ).x != sequence.getValue( 0.5f ).y );
    REQUIRE( sequence.getValue( 0.5f ).y != sequence.getValue( 0.5f ).z );
    REQUIRE( sequence.getValue( 0.5f ).z != sequence.getValue( 0.5f ).w );
  }

  SECTION( "Over- and Under-fill Separate Easings" ) {
    auto sequence = createSequence( vec3( 10.0f ) );
    // The real test here is that we don't crash when creating or using this sequence.
    sequence->then<RampTo3>( vec3( 1.0f ), 1.0f, EaseInOutQuad() ).then<RampTo3>( vec3( 5.0f ), 1.0f, EaseInQuad(), EaseNone(), EaseInAtan(), EaseInBack() );

    REQUIRE( sequence->getValue( 0.0f ).x == sequence->getValue( 0.0f ).y );
    REQUIRE( sequence->getValue( 1.5f ).x != sequence->getValue( 1.5f ).y );
    REQUIRE( sequence->getValue( 1.5f ).y != sequence->getValue( 1.5f ).z );
  }

  SECTION( "Mixing Sequences" ) {
    Sequence<vec2> sequence( vec2( 0 ) );

    Sequence<vec2> bounce_y( vec2( 0 ) );
    bounce_y.then<RampTo>( vec2( 0, 10.0f ), 0.5f, EaseOutCubic() ).then<RampTo>( vec2( 0 ), 0.5f, EaseInCubic() );

    Sequence<vec2> slide_x( vec2( 0 ) );
    slide_x.then<RampTo>( vec2( 100.0f, 0.0f ), 3.0f, EaseOutQuad() );

    auto combine = makeAccumulator( vec2( 0 ), slide_x.asPhrase(), makeRepeat<vec2>( bounce_y.asPhrase(), 3 ) );
    combine->add( bounce_y.asPhrase() );

    sequence.then( combine );

    REQUIRE( sequence.getValue( 0.5 ).y == 20.0 ); // both bounces up
    REQUIRE( sequence.getValue( 1.5 ).y == 10.0 ); // only the repeated bounce up remains
    REQUIRE( sequence.getEndValue().x == 100.0 );
  }
} // Separate Component Easing

#endif
