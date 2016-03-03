//
//  Grouping_test.cpp
//
//  Created by Soso Limited on 10/26/15.
//
//

#include "catch.hpp"
#include "choreograph/Choreograph.h"

using namespace choreograph;
using namespace std;

TEST_CASE( "Motion Groups" )
{
  auto group = std::make_shared<Timeline>();
  group->setDefaultRemoveOnFinish( false );
  auto &group_timeline = *group;
  Output<int> target = 0;
  Timeline timeline;

  SECTION( "Timelines are composable." )
  {
    auto t2 = ch::detail::make_unique<Timeline>();
    int receiver = 0;
    t2->apply( &target )
      .then<RampTo>( 50, 1.0f );
    t2->cue( [&receiver] {
        receiver = 100;
      }, 0.4f );

    timeline.add( std::move( t2 ) );
    timeline.step( 0.5f );

    REQUIRE( target() == 25 );
    REQUIRE( receiver == 100 );
  }

  SECTION( "Motion Group member callbacks are still called." )
  {
    int start_count = 0;
    int update_count = 0;
    int finish_count = 0;

    group_timeline.apply( &target )
      .then<RampTo>( 10, 1.0f )
      .startFn( [&start_count] () {
        start_count += 1;
      } )
      .updateFn( [&update_count] () {
        update_count += 1;
      } )
      .finishFn( [&finish_count] () {
        finish_count += 1;
      } );

    SECTION( "Sanity Check" )
    {
      REQUIRE( group->getDuration() == 1.0f );
    }

    SECTION( "Looping the group still calls child callbacks." )
    {
      group->setFinishFn( [&group_timeline] () {
        group_timeline.resetTime();
      } );
      timeline.setDefaultRemoveOnFinish( false );
      timeline.addShared( group );

      for( int i = 0; i < 32; i += 1 ) {
        timeline.step( 0.1f );
      }

      REQUIRE( start_count == 4 );
      REQUIRE( update_count == 32 );
      REQUIRE( finish_count == 3 );
    }

    SECTION( "Ping-pong looping a group works, too." )
    {
      group->setFinishFn( [&group_timeline] () {
        // MotionGroup overrides customSetPlaybackSpeed to inform children.
        group_timeline.setPlaybackSpeed( -1 * group_timeline.getPlaybackSpeed() );
        group_timeline.resetTime();
      } );
      timeline.setDefaultRemoveOnFinish( false );
      timeline.addShared( group );

      for( int i = 0; i < 32; i += 1 ) {
        timeline.step( 0.1f );
      }

      // We only pass the start and finish of our motion going forward twice.
      // We also pass them each going backward, but that doesn't trigger our functions.
      REQUIRE( start_count == 2 );
      REQUIRE( update_count == 32 );
      REQUIRE( finish_count == 2 );
    }
  } // member callbacks
}
