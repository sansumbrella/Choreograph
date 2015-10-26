
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
// Motions
//==========================================

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
