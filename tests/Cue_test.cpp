//
//  Cue_test.cpp
//
//  Created by Soso Limited on 10/26/15.
//
//

#include "catch.hpp"
#include "choreograph/Choreograph.h"

using namespace choreograph;
using namespace std;

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

  SECTION( "Cues at time zero are called." )
  {
    auto cc = 0;
    timeline.cue( [&cc] { cc += 1; }, 0 );
    timeline.step( 0.1 );

    REQUIRE( cc == 1 );
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

      timeline.jumpTo( -1.1f );
      timeline.jumpTo( -0.5f );
      timeline.jumpTo( -1.1f );

      REQUIRE( call_count == 1 );
    }
  }
}
