//
//  ForumMiscellany_test.cpp
//
//  Created by Soso Limited on 10/26/15.
//
//

#include "catch.hpp"
#include "choreograph/Choreograph.h"

using namespace choreograph;
using namespace std;

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
