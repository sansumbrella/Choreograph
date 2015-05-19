
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "choreograph/Choreograph.h"
#include <array>

// If true, will test vec2 and vec3 animation.
// You must have GLM in the search path for this to work.
#define TEST_WITH_GLM_VECTORS 0

using namespace std;
using namespace choreograph;

//==========================================
// Phrases
//==========================================

TEST_CASE( "Phrases" )
{
  auto ramp = makeRamp( 1.0f, 10.0f, 1.0f );
  auto other = makeRamp( 10.0f, 100.0f, 1.0f );
  auto hold = make_shared<Hold<float>>( 5.0f, 1.0f );

  SECTION( "Ramps perform linear interpolation between two values." )
  {
    REQUIRE( ramp->getValue( 0.0f ) == 1.0f );
    REQUIRE( ramp->getValue( 0.5f ) == 5.5f );
    REQUIRE( ramp->getValue( 1.0f ) == 10.0f );
  }

  SECTION( "Holds return a single value over a duration." )
  {
    REQUIRE( hold->getValue( 0.2f ) == hold->getValue( 1.0f ) );
  }

  SECTION( "Retime phrases change how an existing phrase plays back." )
  {
    auto repeat = makeRepeat<float>( ramp, 4 );
    auto reverse = makeReverse<float>( ramp );
    auto ping = makePingPong<float>( ramp, 7 );

    REQUIRE( reverse->getValue( 0.2 ) == ramp->getValue( 0.8 ) );
    REQUIRE( repeat->getValue( 3 ) == ramp->getValue( 0 ) );
    REQUIRE( ping->getValue( 1 ) == ramp->getValue( 1 ) );
    REQUIRE( ping->getValue( 2 ) == ramp->getValue( 0 ) );

    REQUIRE( repeat->getDuration() == ramp->getDuration() * 4 );
    REQUIRE( reverse->getDuration() == ramp->getDuration() );
    REQUIRE( ping->getDuration() == ramp->getDuration() * 7 );
  }

  SECTION( "Procedural phrases return values from functional procedures." )
  {
    auto proc = makeProcedure<float>( 1.0f, [] ( Time t, Time duration ) {
      return sin( t * PI ) * 10.0f;
    } );

    REQUIRE( proc->getValue( 0 ) == proc->getStartValue() );
    REQUIRE( proc->getValue( 1 ) == proc->getEndValue() );
    REQUIRE( proc->getValue( 0.5 ) == 10 );
    REQUIRE( proc->getValue( 1 ) < 1.0e-14 );
  }

  SECTION( "Mix phrases blend between two phrases." )
  {
    auto mix_0 = makeBlend<float>( ramp, other, 0.0f );
    auto mix_25 = makeBlend<float>( ramp, other, 0.25f );
    auto mix_50 = makeBlend<float>( ramp, other, 0.5f );

    auto mix_100 = makeBlend<float>( ramp, other );
    mix_100->setMix( 1.0f );

    REQUIRE( mix_100->getMix() == 1.0f );

    REQUIRE( mix_0->getValue( 1.0f ) == ramp->getValue( 1.0f ) );
    REQUIRE( mix_25->getValue( 1.0f ) == 32.5f );
    REQUIRE( mix_50->getValue( 1.0f ) == 55.0f );
    REQUIRE( mix_100->getValue( 1.0f ) == other->getValue( 1.0f ) );
  }

  SECTION( "Accumulate phrases mix the output of multiple phrases into one." )
  {
    auto accumulate = makeAccumulator<float>( 0.0f, ramp, other );

    auto sum = makeAccumulator<float>( 10.0f, ramp );

    auto decumulate = makeAccumulator<float>( 0.0f, ramp, other, [] (float a, float b) {
      return a - b;
    } );

    REQUIRE( accumulate->getValue( 1.0 ) == 110 );
    REQUIRE( sum->getValue( 1.0 ) == 20 );
    REQUIRE( decumulate->getValue( 1.0 ) == -110 );
  }

  SECTION( "Ramps and Mix Phrases receive optional interpolation functions to support custom objects." )
  {
    struct Obj {
      float       x;
      float       y;
      std::string name;
    };

    auto lerp_obj = [] (const Obj &lhs, const Obj &rhs, float mix) {
      return Obj{ ch::lerpT( lhs.x, rhs.x, mix ),
                  ch::lerpT( lhs.y, rhs.y, mix ),
                  lhs.name };
    };

    auto a = Obj{ 0.0f, 10.0f, "hello" };
    auto b = Obj{ 10.0f, 100.0f, "target" };
    auto c = Obj{ 100.0f, 1000.0f, "another" };

    auto ramp_ab = makeRamp( a, b, 1.0f, EaseInOutQuad(), lerp_obj );
    auto ramp_bc = makeRamp( b, c, 1.0f, EaseNone(), lerp_obj );

    auto mix_ramps = makeBlend<Obj>( ramp_ab, ramp_bc, 0.5f, lerp_obj );

    REQUIRE( ramp_ab->getValue( 1.0f ).x == b.x );
    REQUIRE( ramp_ab->getValue( 0.5f ).x == 5.0f );
    REQUIRE( ramp_ab->getValue( 0.5f ).y == 55.0f );
    REQUIRE( ramp_ab->getValue( 1.0f ).name == "hello" );
    REQUIRE( ramp_bc->getValue( 1.0f ).name == "target" );
    REQUIRE( mix_ramps->getValue( 0.5f ).y == ((550.0f * 0.5f) + (55.0f * 0.5f)) );
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

  SECTION( "Sequence duration is the sum of all of its component phrase's durations." )
  {
    REQUIRE( sequence.getDuration() == 3 );
  }

  SECTION( "Sequences have multiple construction patterns." )
  {
    SECTION( "Copy Constructor" )
    {
      Sequence<float> other( sequence );

      REQUIRE( other.getDuration() == sequence.getDuration() );
      REQUIRE( other.getValue( 1.5 ) == sequence.getValue( 1.5 ) );
    }

    SECTION( "Phrase Constructor" )
    {
      // Bug in VS2013 makes the single ref convertible to a vector ref.
      // Casting to base type avoids that incorrect conversion.
      auto ramp = PhraseRef<float>( makeRamp( 1.0f, 10.0f, 1.0f ) );
      Sequence<float> other( ramp );

      REQUIRE( other.getValue( 0.5 ) == ramp->getValue( 0.5 ) );
      REQUIRE( other.getDuration() == ramp->getDuration() );
    }
  }

  SECTION( "Sequence values within duration are interpolated by the appropriate Phrase." )
  {
    REQUIRE( sequence.getValue( 0.5 ) == 0.5 );
    REQUIRE( sequence.getValue( 1.0 ) == 1.0 );
    REQUIRE( sequence.getValue( 1.5 ) == 5.5 );
  }

  SECTION( "Sequence values outside duration are clamped to begin and end values." )
  {
    REQUIRE( sequence.getValue( - std::numeric_limits<float>::max() ) == sequence.getStartValue() );
    REQUIRE( sequence.getValue( std::numeric_limits<float>::max() ) == sequence.getEndValue() );
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
    // The code generated by VS2013 differs a bit from Clang, so
    // we explicitly create the sequence phrase first to guarantee what kind of sequence we get.
    auto phrase = sequence.asPhrase();
    sequence.then( sequence ).then( phrase );

    // We have the sequence three times.
    REQUIRE( sequence.getDuration() == 9.0f );
    REQUIRE( sequence.getValue( 3.5f ) == sequence.getValue( 6.5f ) );
    REQUIRE( sequence.getValue( 1.0f ) == sequence.getValue( 4.0f ) );
  }

  SECTION( "Phrases can be spliced into sequences." )
  {
    REQUIRE( sequence.size() == 3 );
    sequence.splice( 1, 1, {} ); // 0-1, 10-100
    REQUIRE( sequence.size() == 2 );

    auto phrase = makeRamp( 10.0f, 50.0f, 1.0 );
    auto another = makeReverse<float>( phrase );
    sequence.splice( 1, 0, { phrase, another } ); // 0-1, 10-50, 50-10, 10-100
    REQUIRE( sequence.size() == 4 );
    REQUIRE( sequence.getDuration() == 4.0 );
    REQUIRE( sequence.getValue( 2.0 ) == 50.0f );

    phrase->setEndValue( 500.0f );
    REQUIRE( sequence.getValue( 2.0 ) == 500.0f );
    REQUIRE( sequence.getPhraseAtTime( 1.8 ) == phrase );
    REQUIRE( sequence.getPhraseAtIndex( 2 ) == another );

    sequence.replacePhraseAtIndex( 2, sequence.getPhraseAtIndex( 1 ) );
    REQUIRE( sequence.size() == 4 );
    REQUIRE( sequence.getPhraseAtIndex( 1 ) == sequence.getPhraseAtIndex( 2 ) );
  }

  SECTION( "Sequences prevent incorrect splicing." )
  {
    sequence.splice( 100, 100, {} );
    REQUIRE( sequence.size() == 3 );

    sequence.splice( 0, 100, {} );
    REQUIRE( sequence.size() == 0 );
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

  SECTION( "Motion duration is derived from its Sequence." )
  {
    REQUIRE( motion.getDuration() == sequence.getDuration() );
    REQUIRE( motion.getDuration() == 3 );
  }

  SECTION( "Motions apply their Sequence's value to their target." )
  {
    vector<Time> times = { 0.5, 0.2f, 1.0, 0.0, 2.0, 2.5, 3.0, 0.0, 0.3f, 0.5 };
    for( auto &t : times ) {
      motion.jumpTo( t );
      REQUIRE( target() == sequence.getValue( t ) );
    }
  }
}

TEST_CASE( "Motion Groups" )
{
  auto group = detail::make_unique<MotionGroup>();
  auto &group_timeline = group->timeline();
  Output<int> target = 0;
  Timeline timeline;

  SECTION( "Timelines are composable." )
  {
    Timeline t2;
    int receiver = 0;
    t2.apply( &target )
      .then<RampTo>( 50, 1.0f );
    t2.cue( [&receiver] {
        receiver = 100;
      }, 0.4f );

    timeline.add( std::move( t2 ) );
    timeline.step( 0.5f );

    REQUIRE( target == 25 );
    REQUIRE( receiver == 100 );
  }

  SECTION( "Motion Group member callbacks are still called." )
  {
    int start_count = 0;
    int update_count = 0;
    int finish_count = 0;

    group_timeline.apply( &target )
      .then<RampTo>( 10, 1.0f )
      .startFn( [&start_count] (Motion<int> &m) {
        start_count += 1;
      } )
      .updateFn( [&update_count] (Motion<int> &m) {
        update_count += 1;
      } )
      .finishFn( [&finish_count] (Motion<int> &m) {
        finish_count += 1;
      } );

    SECTION( "Sanity Check" )
    {
      REQUIRE( group->getDuration() == 1.0f );
    }

    SECTION( "Looping the group still calls child callbacks." )
    {
      group->setFinishFn( [] ( MotionGroup &g ) {
        g.resetTime();
      } );
      timeline.setDefaultRemoveOnFinish( false );
      timeline.add( std::move( group ) );

      for( int i = 0; i < 32; i += 1 ) {
        timeline.step( 0.1f );
      }

      REQUIRE( start_count == 4 );
      REQUIRE( finish_count == 3 );
      REQUIRE( update_count == 32 );
    }

    SECTION( "Ping-pong looping group works, too." )
    {
      group->setFinishFn( [] ( MotionGroup &g ) {
        // MotionGroup overrides customSetPlaybackSpeed to inform children.
        g.setPlaybackSpeed( -1 * g.getPlaybackSpeed() );
        g.resetTime();
      } );
      timeline.setDefaultRemoveOnFinish( false );
      timeline.add( std::move( group ) );

      for( int i = 0; i < 32; i += 1 ) {
        timeline.step( 0.1f );
      }

      REQUIRE( start_count == 4 );
      REQUIRE( finish_count == 3 );
      REQUIRE( update_count == 32 );
    }

  }
}

TEST_CASE( "Forum Questions" )
{
  SECTION("Moving an Output updates the Motion target.")
  {
    // Based on Cinder forum issue:
    // https://forum.libcinder.org/topic/bug-hang-in-timeline-replacetarget-function-infinite-loop#23286000002274021
    ch::Timeline timeline;

    ch::Output<float> oFloat[2] = { 0.0f, 0.0f };
    timeline.apply( &oFloat[0] ).then<RampTo>( 1.0f, 1.0f );
    timeline.apply( &oFloat[0] ).then<RampTo>( 2.0f, 1.0f ).removeOnFinish(true);
    oFloat[1] = std::move(oFloat[0]);

    timeline.jumpTo(1.0f);

    REQUIRE(timeline.size() == 0);
    REQUIRE(oFloat[0].value() == 0.0f);
    REQUIRE(oFloat[1].value() == 2.0f);
  }
}

//==========================================
// Slicing and dicing
//==========================================

TEST_CASE( "Slicing Time" )
{
  SECTION( "Clip Phrases retime existing phrases and clamp their end values." )
  {
    auto ramp = makeRamp( 1.0f, 10.0f, 1.0f );
    auto clip_equal = ClipPhrase<float>( ramp, 0.0f, 1.0f );
    auto clip_from_start = ClipPhrase<float>( ramp, 0.0f, 0.5f );
    auto clip_middle = ClipPhrase<float>( ramp, 0.25f, 0.75f );
    auto clip_past_end = ClipPhrase<float>( ramp, 0.5f, 1.25f );

    REQUIRE( clip_equal.getDuration() == 1.0f );
    REQUIRE( clip_equal.getStartValue() == ramp->getStartValue() );
    REQUIRE( clip_equal.getEndValue() == ramp->getEndValue() );
    REQUIRE( clip_equal.getValue( 0.5f ) == ramp->getValue( 0.5f ) );

    REQUIRE( clip_from_start.getDuration() == 0.5f );
    REQUIRE( clip_from_start.getValue( 0.0f ) == ramp->getValue( 0.0f ) );
    REQUIRE( clip_from_start.getValue( 10.0f ) == ramp->getValue( 0.5f ) );

    REQUIRE( clip_middle.getDuration() == 0.5f );
    REQUIRE( clip_middle.getValue( 0.0f ) == ramp->getValue( 0.25f ) );
    REQUIRE( clip_middle.getEndValue() == ramp->getValue( 0.75f ) );

    REQUIRE( clip_past_end.getDuration() == 0.75f );
    REQUIRE( clip_past_end.getEndValue() == ramp->getEndValue() );
    REQUIRE( clip_past_end.getValue( 0.5f ) == ramp->getValue( 1.0f ) );
  }

  Output<float> target = 0.0f;
  auto sequence = Sequence<float>( 0.0f )
    .then<RampTo>( 1.0f, 1.0f )
    .then<RampTo>( 10.0f, 1.0f )
    .then<RampTo>( 100.0f, 1.0f );

  SECTION( "Sequences can be sliced into subsequences." )
  {
    auto slice_equal = sequence.slice( 0.0f, sequence.calcDuration() );
    auto slice_middle = sequence.slice( 0.25f, 2.25f );
    auto slice_past_end = sequence.slice( 0.5f, 3.5f );

    REQUIRE( slice_equal.getDuration() == sequence.getDuration() );
    REQUIRE( slice_equal.getValue( 1.5f ) == sequence.getValue( 1.5f ) );

    REQUIRE( sequence.getDuration() == 3 );
    REQUIRE( slice_middle.getDuration() == 2 );
    REQUIRE( slice_middle.getValue( 0.0f ) == sequence.getValue( 0.25f ) );
    REQUIRE( slice_middle.getEndValue() == sequence.getValue( 2.25f ) );

    REQUIRE( slice_past_end.getDuration() == 3 );
    REQUIRE( slice_past_end.getValue( 3.0f ) == sequence.getEndValue() );
    REQUIRE( slice_past_end.getValue( 0.0f ) == sequence.getValue( 0.5f ) );
  }


  SECTION( "Motions can slice their Sequence." )
  {
    Motion<float> motion( &target, sequence );

    SECTION( "Slicing changes the Sequence animation." )
    {
      motion.sliceSequence( 0.5f, 1.5f );

      motion.jumpTo( 1.0f );
      float v1 = target();

      motion.jumpTo( 0.0f );
      float v2 = target();

      REQUIRE( motion.getDuration() == 1 );
      REQUIRE( v1 == sequence.getValue( 1.5f ) );
      REQUIRE( v2 == sequence.getValue( 0.5f ) );
    }

    SECTION( "When sliced, the Motion's time is adjusted to be fixed relative to its Sequence." )
    {
      motion.jumpTo( 1.0f );
      motion.sliceSequence( 0.5f, 1.5f );

      REQUIRE( motion.time() == 0.5f );
    }

    SECTION( "Cutting before removes past Phrases from the Sequence." )
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

    SECTION( "Cut In slices the Sequence so it ends in the specified amount of time." )
    {
      motion.cutIn( 2.0f );
      REQUIRE( motion.getDuration() == 2 );
    }

    SECTION( "Cut In also removes Phrases prior to the current Motion time." )
    {
      motion.jumpTo( 1.0f );
      motion.cutIn( 2.0f );

      REQUIRE( motion.time() == 0 );
      REQUIRE( motion.getDuration() == 2 );
    }

    SECTION( "Cut In will extend the Sequence if the cut time is past the end of the Sequence." )
    {
      motion.jumpTo( 2.5f );
      motion.cutIn( 2.0f );

      motion.jumpTo( 1.5f );
      auto v1 = target();

      REQUIRE( v1 == sequence.getEndValue() );
      REQUIRE( motion.getDuration() == 2 );
    }
  }

  SECTION( "Timeline MotionOptions expose trim methods." )
  {
    Timeline timeline;
    auto options = timeline.apply( &target, sequence );

    SECTION( "With single Motion, Timeline duration is motions duration." )
    {
      REQUIRE( timeline.timeUntilFinish() == sequence.getDuration() );
    }

    SECTION( "Cut at trims the sequence relative to their start." )
    {
      options.cutAt( 2.0f );
      REQUIRE( timeline.timeUntilFinish() == 2.0f );
    }

    SECTION( "Cut In trims the sequence relative to the motion's current time." )
    {
      timeline.step( 0.5f );
      float v1 = target();
      options.cutIn( 0.5f );
      timeline.step( 0.0f );
      float v2 = target();

      REQUIRE( timeline.timeUntilFinish() == 0.5f );
      REQUIRE( v1 == v2 );
    }
  }
}

//==========================================
// Cues
//==========================================

TEST_CASE( "Cues" )
{
  Timeline  timeline;
  int       call_count = 0;
  auto      options = timeline.cue( [&call_count] { call_count += 1; }, 1.0f );

  SECTION( "Cues are called on time." )
  {
    timeline.jumpTo( 0.5f );
    REQUIRE( call_count == 0 );

    timeline.jumpTo( 1.0f );
    REQUIRE( call_count == 1 );
  }

  SECTION( "Cues can be cancelled by Handle." )
  {
    auto handle = options.getControl();
    handle->cancel();
    timeline.jumpTo( 1.0f );
    REQUIRE( call_count == 0 );
  }

  SECTION( "Living Scoped Control allows cue to call." )
  {
    auto scoped_control = options.getScopedControl();
    timeline.jumpTo( 1.0f );
    REQUIRE( call_count == 1 );
  }

  SECTION( "Destructed Scoped Control prevents cue from calling." )
  {
    {
      auto scoped_control = options.getScopedControl();
    }
    timeline.jumpTo( 1.0f );
    REQUIRE( call_count == 0 );
  }

  SECTION( "Cue signalling is directional." )
  {
    options.removeOnFinish( false );

    SECTION( "Forward" )
    {
      timeline.jumpTo( 1.0f );
      timeline.jumpTo( 0.5f );
      timeline.jumpTo( 1.0f );

      REQUIRE( call_count == 2 );
    }

    SECTION( "Reversed" )
    {
      options.playbackSpeed( -1 );

      timeline.jumpTo( 1.1f );
      timeline.jumpTo( 0.5f );
      timeline.jumpTo( 1.1f );

      REQUIRE( call_count == 1 );
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

    options.startFn( [&startCalled] (Motion<float> &) { startCalled = true; } )
      .updateFn( [&updateTarget, &target, &updateCount] ( Motion<float> &m ) { updateTarget = target() / 2.0f; updateCount++; } )
      .finishFn( [&endCalled] (Motion<float> &) { endCalled = true; } );

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
    bool triggered = false;

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

    timeline.append( &target )
      .onInflection( [&triggered] (Motion<float> &m) { triggered = true; } )
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
    REQUIRE( triggered );
  }

  SECTION( "It is safe to add and cancel motions from Cues and Motion callbacks." )
  {
    Output<float> t2 = 1.0f;

    SECTION( "Add Motion from Motion callback." )
    {
      options.startFn( [&] (Motion<float> &) {
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
        .finishFn( [&target] (Motion<float> &) {
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
    self_destructing_timeline->setFinishFn( [&self_destructing_timeline] {
      self_destructing_timeline.reset();
    } );

    REQUIRE( self_destructing_timeline );

    // Stepping forward gets to the end and triggers our finish fn.
    // Note that our application does not crash here (that's the real test).
    self_destructing_timeline->jumpTo( sequence.getDuration() );

    REQUIRE_FALSE( self_destructing_timeline );
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
      REQUIRE( ! motion->cancelled() );
      REQUIRE( temp.isConnected() );
    }

    REQUIRE( motion->cancelled() );
  }

  SECTION( "Disconnecting outputs works." )
  {
    Output<float> temp = 0.0f;
    REQUIRE_FALSE( temp.isConnected() );
    temp.disconnect();
    Motion<float> m( &temp );
    REQUIRE( temp.isConnected() );
    temp.disconnect();
    REQUIRE_FALSE( temp.isConnected() );

  }

  SECTION( "Motion falling out of scope disconnects" )
  {
    { // hook up a motion to our output
      Motion<float> temp( &output );

      REQUIRE( output.isConnected() == true );
      REQUIRE( temp.cancelled() == false );
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
    vector<Output<float>> outputs;
    vector<Output<float>> copy;

    while( outputs.size() < 500 ) {
      outputs.emplace_back( Output<float>{ 0.0f } );
    }

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
}

//========================================================
// Separate Component Interpolation (with glm::vec types)
//========================================================

#if TEST_WITH_GLM_VECTORS

#include "glm/glm.hpp"
#include <array>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;

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
    Sequence<vec3> sequence( vec3(10.0f) );
    // The real test here is that we don't crash when creating or using this sequence.
    sequence.then<RampTo3>( vec3( 1.0f ), 1.0f, EaseInOutQuad() ).then<RampTo3>( vec3( 5.0f ), 1.0f, EaseInQuad(), EaseNone(), EaseInAtan(), EaseInBack() );

    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).y );
    REQUIRE( sequence.getValue( 1.5f ).x != sequence.getValue( 1.5f ).y );
    REQUIRE( sequence.getValue( 1.5f ).y != sequence.getValue( 1.5f ).z );
  }
} // Separate Component Easing

#endif
