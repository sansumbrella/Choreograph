
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "choreograph/Choreograph.hpp"

// If true, will test vec2 and vec3 animation and different ease functions.
#define INCLUDE_CINDER_HEADERS 1

using namespace std;
using namespace choreograph;

TEST_CASE( "Sequence Interpolation", "[sequence]" ) {
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

    REQUIRE( (sequence.wrapTime( 10 * sequence.getDuration() + offset ) - offset) < epsilon );
    REQUIRE( (sequence.getValueWrapped( sequence.getDuration() + offset ) - sequence.getValue( offset ) ) < epsilon );
    REQUIRE( (sequence.getValueWrapped( (2 * sequence.getDuration()) + offset ) - sequence.getValue( offset ) ) < epsilon );
    REQUIRE( (sequence.getValueWrapped( (50 * sequence.getDuration()) + offset ) - sequence.getValue( offset ) ) < epsilon );
  }
}

TEST_CASE( "Raw Pointers" ) {
  float target = 0.0f;
  Timeline timeline;

  SECTION( "Composing Sequences in Sequences" ) {
    auto sequence = createSequence( 1.0f );
    SequenceRef<float> continuation = createSequence( 5.0f );
    continuation->then<RampTo>( 10.0f, 1.0f ).then<Hold>( 3.0f, 1.0f );

    sequence->then<RampTo>( 5.0f, 0.5f ).then( *continuation ).then( continuation );

    REQUIRE( sequence->getDuration() == 4.5f );

    Motion<float> motion( &target, sequence );
    motion.jumpTo( 1.0f );
    REQUIRE( target == 7.5f );

    motion.jumpTo( 1.5f );
    REQUIRE( target == 10.0f );

    timeline.apply( &target, sequence );
    REQUIRE( motion.isValid() == true ); // Working with raw pointer, no management will have happened.

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

TEST_CASE( "Sequence Composition", "[sequence]" ) {
  Output<float> target = 0.0f;
  Timeline timeline;

  SECTION( "Composing Sequences in Sequences" ) {
    auto sequence = createSequence( 1.0f );
    SequenceRef<float> continuation = createSequence( 5.0f );
    continuation->then<RampTo>( 10.0f, 1.0f ).then<Hold>( 3.0f, 1.0f );

    sequence->then<RampTo>( 5.0f, 0.5f ).then( *continuation ).then( continuation );

    REQUIRE( sequence->getDuration() == 4.5f );

    Motion<float> motion( &target, sequence );
    motion.jumpTo( 1.0f );
    REQUIRE( target == 7.5f );

    motion.jumpTo( 1.5f );
    REQUIRE( target == 10.0f );

    timeline.apply( &target, sequence );
    REQUIRE( motion.isValid() == false ); // management will invalidate the motion (because it's been superseded).

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

TEST_CASE( "Output Connections", "[output]" ) {

  co::Timeline timeline;
  auto sequence = make_shared<Sequence<float>>( 0.0f );
  sequence->then<RampTo>( 10.0f, 2.0f );

  SECTION( "Output falling out of scope disconnects" ) {
    MotionRef<float> motion;

    { // create locally scoped output
      Output<float> temp;
      motion = make_shared<Motion<float>>( &temp, sequence );
      REQUIRE( motion->isValid() );
      REQUIRE( temp.isConnected() );
    }

    REQUIRE( ! motion->isValid() );
  }

  SECTION( "Motion falling out of scope disconnects" ) {
    Output<float> output;

    { // hook up a motion to our output
      Motion<float> temp( &output, sequence );

      REQUIRE( output.isConnected() == true );
      REQUIRE( temp.isValid() == true );
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
}

#if INCLUDE_CINDER_HEADERS

#include "cinder/Vector.h"
#include "cinder/Easing.h"
#include <array>
using namespace cinder;

TEST_CASE( "Separate component interpolation", "[sequence]" ) {


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
    sequence.then<RampTo3>( vec3( 10.0f ), 1.0f, std::array<EaseFn, 3>{ EaseOutQuad(), EaseInQuad(), EaseInOutQuad() } );

    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).y );
    REQUIRE( sequence.getValue( 0.0f ).x == sequence.getValue( 0.0f ).z );

    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).y );
    REQUIRE( sequence.getValue( 1.0f ).x == sequence.getValue( 1.0f ).z );

    REQUIRE( sequence.getValue( 2.0f ).x == sequence.getValue( 2.0f ).y );  // past the end
    REQUIRE( sequence.getValue( 2.0f ).x == sequence.getValue( 2.0f ).z );  // past the end

    REQUIRE( sequence.getValue( 0.5f ).x != sequence.getValue( 0.5f ).y );
    REQUIRE( sequence.getValue( 0.5f ).y != sequence.getValue( 0.5f ).z );
    REQUIRE( sequence.getValue( 0.5f ).x != sequence.getValue( 0.5f ).z );
  }
}

#endif
