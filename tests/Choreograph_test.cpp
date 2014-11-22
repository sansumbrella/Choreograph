
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "choreograph/Choreograph.h"
#include <array>

// If true, will test vec2 and vec3 animation.
#define INCLUDE_CINDER_HEADERS true

using namespace std;
using namespace choreograph;

//==========================================
// Phrases
//==========================================

TEST_CASE( "Phrases" )
{
  auto ramp = makeRamp( 1.0f, 10.0f, 1.0f );
  auto other = makeRamp( 10.0f, 100.0f, 1.0f );

  SECTION( "Retime" )
  {
    auto repeat = makeRepeat<float>( ramp, 4 );
    auto reverse = makeReverse<float>( ramp );
    auto ping = makePingPong<float>( ramp, 4 );

    REQUIRE( reverse->getValue( 0.2 ) == ramp->getValue( 0.8 ) );
    REQUIRE( repeat->getValue( 3 ) == ramp->getValue( 0 ) );
    REQUIRE( ping->getValue( 1 ) == ramp->getValue( 1 ) );
    REQUIRE( ping->getValue( 2 ) == ramp->getValue( 0 ) );
  }

  SECTION( "Procedural" )
  {
    auto proc = makeProcedure<float>( 1.0f, [] ( Time t, Time duration ) {
      return sin( t * PI ) * 10.0f;
    } );

    REQUIRE( proc->getValue( 0.5 ) == 10 );
    REQUIRE( proc->getValue( 1 ) < 1.0e-14 );
  }

  SECTION( "Combine" )
  {
    auto accumulate = makeAccumulator<float>( 0.0f, ramp, other );
    REQUIRE( accumulate->getValue( 1.0 ) == 110 );

    auto decumulate = makeAccumulator<float>( 0.0f, ramp, other, [] (float a, float b) {
      return a - b;
    } );
    REQUIRE( decumulate->getValue( 1.0 ) == -110 );
  }
}

//==========================================
// Sequences
//==========================================

TEST_CASE( "Sequences" )
{
  auto sequence = Sequence<float>( 0.0f )
    .then<RampTo>( 1.0f, 1.0f )
    .then<RampTo>( 10.0f, 1.0f )
    .then<RampTo>( 100.0f, 1.0f );

  SECTION( "Sequence duration is correct." )
  {
    REQUIRE( sequence.getDuration() == 3 );
  }

  SECTION( "Sequences can be constructed other ways" )
  {
    SECTION( "Copy Constructor" )
    {
      Sequence<float> other( sequence );

      REQUIRE( other.getDuration() == sequence.getDuration() );
      REQUIRE( other.getValue( 1.5 ) == sequence.getValue( 1.5 ) );
    }

    SECTION( "Phrase Constructor" )
    {
      auto ramp = makeRamp( 1.0f, 10.0f, 1.0f );
      Sequence<float> other( ramp );

      REQUIRE( other.getValue( 0.5 ) == ramp->getValue( 0.5 ) );
      REQUIRE( other.getDuration() == ramp->getDuration() );
    }
  }

  SECTION( "Sequence values within duration are correct." )
  {
    REQUIRE( sequence.getValue( 0.5 ) == 0.5 );
    REQUIRE( sequence.getValue( 1.0 ) == 1.0 );
    REQUIRE( sequence.getValue( 1.5 ) == 5.5 );
  }

  SECTION( "Sequence values outside duration are clamped." )
  {
    REQUIRE( sequence.getValue( - std::numeric_limits<float>::max() ) == 0.0f );
    REQUIRE( sequence.getValue( std::numeric_limits<float>::max() ) == 100.0f );
  }

  SECTION( "Looped sequence values are equivalent." )
  {
    Time offset = 1.55;
    // Since the precision of floats decreases as they get larger,
    // we need an epsilon larger than the delta from 1.0 to the next float.
    const Time epsilon = 1.0e-5;

    REQUIRE( (wrapTime( 10 * sequence.getDuration() + offset, sequence.getDuration() ) - offset) < epsilon );
    REQUIRE( (sequence.getValueWrapped( sequence.getDuration() + offset ) - sequence.getValue( offset ) ) < epsilon );
    REQUIRE( (sequence.getValueWrapped( (2 * sequence.getDuration()) + offset ) - sequence.getValue( offset ) ) < epsilon );
    REQUIRE( (sequence.getValueWrapped( (20 * sequence.getDuration()) + offset ) - sequence.getValue( offset ) ) < epsilon );
  }

  SECTION( "Sequences are composable." )
  {
    sequence.then( sequence ).then( sequence.asPhrase() );

    // We have the sequence four times, since it was doubled by the time we add it a second time.
    REQUIRE( sequence.getDuration() == 12.0f );
    REQUIRE( sequence.getValue( 3.5f ) == sequence.getValue( 6.5f ) );
    REQUIRE( sequence.getValue( 1.0f ) == sequence.getValue( 4.0f ) );
  }

  SECTION( "Sequences can be sliced into subsequences." )
  {
    auto sub = sequence.slice( 0.5f, 3.5f );
    auto alt = sequence.slice( 0.25f, 2.25f );

    REQUIRE( sequence.getDuration() == 3 );
    REQUIRE( sub.getDuration() == 3 );
    REQUIRE( alt.getDuration() == 2 );
  }
}

//==========================================
// Motions
//==========================================

TEST_CASE( "Motions" )
{
  Output<float> target = 0.0f;
  auto sequence = Sequence<float>( 0.0f )
    .then<RampTo>( 1.0f, 1.0f )
    .then<RampTo>( 10.0f, 1.0f )
    .then<RampTo>( 100.0f, 1.0f );

  Motion<float> motion( &target, sequence );

  SECTION( "Motion Duration is Correct" )
  {
    REQUIRE( motion.getDuration() == sequence.getDuration() );
    REQUIRE( motion.getDuration() == 3 );
  }

  SECTION( "Slicing Source" )
  {
    SECTION( "Cut Before" )
    {
      motion.jumpTo( 1.5f );
      float v1 = target();
      motion.cutPhrasesBefore( motion.time() );
      motion.jumpTo( 0.0f );
      float v2 = target();
      REQUIRE( v1 == v2 );
      REQUIRE( v1 == 5.5f );
      REQUIRE( motion.getDuration() == 1.5f );
    }

    SECTION( "Slice" )
    {
      motion.sliceSequence( 0.5f, 1.5f );
      motion.jumpTo( 1.5f );
      float v1 = target();
      motion.jumpTo( -1.0f );
      float v2 = target();

      REQUIRE( motion.getDuration() == 1 );
      REQUIRE( v1 == sequence.getValue( 1.5f ) );
      REQUIRE( v2 == sequence.getValue( 0.5f ) );
    }

    SECTION( "Cut In" )
    {
      motion.cutIn( 2.0f );
      REQUIRE( motion.getDuration() == 2 );
    }

    SECTION( "Cut In Later" )
    {
      motion.jumpTo( 1.0f );
      motion.cutIn( 2.0f );
      REQUIRE( motion.getDuration() == 2 );
    }

    SECTION( "Cut Past End" )
    {
      motion.jumpTo( 2.5f );
      motion.cutIn( 2.0f );
      REQUIRE( motion.getDuration() == 2 );
    }
  }
}

//==========================================
// Timeline and Options
//==========================================

TEST_CASE( "Timeline" )
{
  Timeline      timeline;
  Output<float> target = 0.0f;

  auto sequence = Sequence<float>( 0.0f )
    .then<RampTo>( 1.0f, 1.0f )
    .then<RampTo>( 10.0f, 1.0f )
    .then<RampTo>( 100.0f, 1.0f );

  auto options = timeline.apply( &target, sequence );

  SECTION( "Output<T> Pointers" )
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
    REQUIRE( motion.isInvalid() == true );

    timeline.jumpTo( 2.0f );
    REQUIRE( target == 10.0f );
  }

  SECTION( "Raw Pointers" )
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
    REQUIRE( motion.isInvalid() == false );

    timeline.jumpTo( 2.0f );
    REQUIRE( target == 10.0f );
  }

  SECTION( "Equivalence between motion time and sequence time" )
  {
    options.removeOnFinish( false );

    vector<Time> times = { 0.5, 0.2f, 1.0, 0.0, 2.0, 2.5, 3.0, 0.0, 0.3f, 0.5 };
    for( auto &t : times ) {
      timeline.jumpTo( t );
      REQUIRE( target() == sequence.getValue( t ) );
    }
  }

  SECTION( "Inflection Point Callbacks" )
  {
    int c1 = 0;
    int c2 = 0;

    timeline.apply( &target )
      .hold( 0.5f )
      // inflects around 0.5
      .onInflection( [&c1] (Motion<float> &m) { c1 += 1; } )
      .then<RampTo>( 3.0f, 1.0f )
      // inflects around 1.5
      .onInflection( [&c2] (Motion<float> &m) {
        c2 += 1;
      } )
      .then<RampTo>( 2.0f, 1.0f );

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
  }

  SECTION( "Trimming" )
  {
    SECTION( "Cut At" )
    {
      REQUIRE( timeline.calcDuration() == 3.0f );
      options.cutAt( 2.0f );
      REQUIRE( timeline.calcDuration() == 2.0f );
    }

    SECTION( "Cut In" )
    {
      REQUIRE( timeline.calcDuration() == 3.0f );
      options.cutIn( 0.5f );
      REQUIRE( timeline.calcDuration() == 0.5f );
    }
  }

  SECTION( "Motion Callbacks" )
  {
    bool          updateCalled = false;
    bool          startCalled = false;
    bool          endCalled = false;
    Output<float> target = 0;
    int           updateCount = 0;
    float         updateTarget = 0;

    timeline.apply( &target )
      .startFn( [&startCalled] (Motion<float> &) { startCalled = true; } )
      .updateFn( [&updateCalled, &updateTarget, &updateCount] ( float value ) { updateTarget = value / 2.0f; updateCount++; updateCalled = true; } )
      .finishFn( [&endCalled] (Motion<float> &) { endCalled = true; } )
      .then<RampTo>( 10.0f, 1.0f );

    SECTION( "Callbacks from step" )
    {
      timeline.step( 0.1f );
      REQUIRE( startCalled );
      REQUIRE( updateCalled );
      REQUIRE( updateCount == 1 );
      REQUIRE( updateTarget == (target / 2.0f) );
      REQUIRE( target == 1.0f );

      for( int i = 0; i < 9; ++i ) {
        REQUIRE_FALSE( endCalled );
        timeline.step( 0.1f );
      }

      REQUIRE( endCalled );
      REQUIRE( updateCount == 10 );
    }

    SECTION( "Callbacks from jumpTo" )
    {
      timeline.jumpTo( 0.1f );
      REQUIRE( startCalled );
      REQUIRE( updateCalled );
      REQUIRE( updateCount == 1 );
      REQUIRE( updateTarget == (target / 2.0f) );
      REQUIRE( target == 1.0f );
      REQUIRE_FALSE( endCalled );

      timeline.jumpTo( 0.9f );
      REQUIRE( updateCount == 2 );
      REQUIRE_FALSE( endCalled );

      timeline.jumpTo( 1.0f );
      REQUIRE( updateCount == 3 );
      REQUIRE( endCalled );
    }
  }


  SECTION( "Cues" )
  {
    timeline.clear();
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

    REQUIRE( timeline.size() == 1 );
    REQUIRE( call_counts[0] == 1 );
    REQUIRE( call_counts[1] == 1 );
    REQUIRE( call_counts[2] == 1 );
    REQUIRE( call_counts[3] == 0 );

    for( int i = 0; i < 11; ++i ) {
      timeline.step( 0.1f );
    }

    REQUIRE( timeline.size() == 0 );
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

    SECTION( "Scoped Control, destructed" )
    {
      {
        auto handle = timeline.cue( [&call_count] { call_count += 1; }, 1.0f ).getScopedControl();
      }
      timeline.jumpTo( 1.0f );
      REQUIRE( call_count == 0 );
    }

    SECTION( "Scoped Control, persistent" )
    {
      ScopedCueRef cue;
      {
        cue = timeline.cue( [&call_count] { call_count += 1; }, 1.0f ).getScopedControl();
      }
      timeline.jumpTo( 1.0f );
      REQUIRE( call_count == 1 );
    }
  }

  SECTION( "Manipulating During Update" )
  {
    SECTION( "Add Motion From Cue" )
    {

    }

    SECTION( "Cancel Motion From Cue" )
    {

    }
  }
}

//==========================================
// Output
//==========================================

TEST_CASE( "Outputs" )
{
  ch::Timeline  timeline;
  Output<float> output = 0.0f;
  auto sequence = Sequence<float>( 0.0f )
    .then<RampTo>( 1.0f, 1.0f )
    .then<RampTo>( 10.0f, 1.0f );

  SECTION( "Output falling out of scope disconnects" )
  {
    // Reference so we can survive the inner scope.
    MotionRef<float> motion;

    { // create locally scoped output
      Output<float> temp = 0.0f;
      motion = make_shared<Motion<float>>( &temp );
      REQUIRE( ! motion->isInvalid() );
      REQUIRE( temp.isConnected() );
    }

    REQUIRE( motion->isInvalid() );
  }

  SECTION( "Motion falling out of scope disconnects" )
  {
    { // hook up a motion to our output
      Motion<float> temp( &output );

      REQUIRE( output.isConnected() == true );
      REQUIRE( temp.isInvalid() == false );
    }

    REQUIRE( output.isConnected() == false );
  }

  SECTION( "Timeline Removes Invalid Connections" )
  {
    { // create locally scoped output
      Output<float> temp;
      timeline.apply( &temp ).then<RampTo>( 5.0f, 1.0f );

      REQUIRE( timeline.size() == 1 );
    }

    // Part of test is that nothing fails when stepping the timeline.
    timeline.step( 0.5f );
    REQUIRE( timeline.empty() == true );
  }

  SECTION( "Vector of outputs can be moved." )
  {
    vector<Output<float>> outputs( 500, 0.0f );
    vector<Output<float>> copy;

    for( auto &output : outputs ) {
      timeline.apply( &output, sequence );
    }
    copy = std::move( outputs );

    timeline.step( 1.0f );
    bool all_five = true;
    for( auto &c : copy ) {
      if( c != 1.0f ) {
        all_five = false;
      }
    }
    REQUIRE( copy.front() == 1.0f );
    REQUIRE( all_five == true );
  }

  SECTION( "Move assignment brings motion along." )
  {
    Output<float> base( 1.0f );
    Output<float> copy( 0.0f );

    Motion<float> motion( &base, sequence );
    copy = std::move( base );
    motion.jumpTo( 1.0f );

    REQUIRE( copy.value() == 1.0f );

    motion.jumpTo( 2.0f );
    REQUIRE( copy.value() == 10.0f );
  }

  SECTION( "Vector of outputs can be copied." )
  {
    vector<Output<float>> outputs( 500, 0.0f );
    vector<Output<float>> copy;

    for( auto &output : outputs ) {
      timeline.apply( &output, sequence );
    }
    copy = outputs;

    timeline.step( 1.0f );
    bool all_five = true;
    for( auto &c : copy ) {
      if( c != 1.0f ) {
        all_five = false;
      }
    }
    REQUIRE( copy.front() == 1.0f );
    REQUIRE( all_five == true );
    REQUIRE( outputs.front() == 0.0f );
  }

  SECTION( "Copy assignment brings motion along. (this may be removed in future)" )
  {
    Output<float> base( 500.0f );
    Output<float> copy( 0.0f );

    Motion<float> motion( &base, sequence );
    copy = base;
    motion.jumpTo( 1.0f );

    REQUIRE( base.value() == 500.0f );
    REQUIRE( copy.value() == 1.0f );

    motion.jumpTo( 2.0f );
    REQUIRE( copy.value() == 10.0f );
  }
} // Outputs

//==========================================
// Time and Other Stuff
//==========================================

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

#if INCLUDE_CINDER_HEADERS

#include "cinder/Vector.h"
#include <array>
using namespace cinder;

TEST_CASE( "Separate component interpolation", "[sequence]" )
{

  SECTION( "Compare 2-Component Values" )
  {
    // Animate XY from and to same values with different ease curves.
    Sequence<vec2> sequence( vec2( 1 ) );
    sequence.then<RampTo2>( vec2( 10.0f ), 1.0f, EaseOutQuad(), EaseInQuad() );
    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).y );
    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).y );
    REQUIRE( sequence.getValue( 2.0f ).x == sequence.getValue( 2.0f ).y );  // past the end
    REQUIRE( sequence.getValue( 0.5f ).x != sequence.getValue( 0.5f ).y );
  }

  SECTION( "Compare 3-Component Values" )
  {
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

  SECTION( "Compare 4-Component Values" )
  {
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

  SECTION( "Over- and Under-fill Separate Easings" )
  {
    auto sequence = createSequence( vec3( 10.0f ) );
    // The real test here is that we don't crash when creating or using this sequence.
    sequence->then<RampTo3>( vec3( 1.0f ), 1.0f, EaseInOutQuad() ).then<RampTo3>( vec3( 5.0f ), 1.0f, EaseInQuad(), EaseNone(), EaseInAtan(), EaseInBack() );

    REQUIRE( sequence->getValue( 0.0f ).x == sequence->getValue( 0.0f ).y );
    REQUIRE( sequence->getValue( 1.5f ).x != sequence->getValue( 1.5f ).y );
    REQUIRE( sequence->getValue( 1.5f ).y != sequence->getValue( 1.5f ).z );
  }

  SECTION( "Mixing Sequences" )
  {
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
