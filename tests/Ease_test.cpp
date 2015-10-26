//
//  Ease_test.cpp
//
//  Created by Soso Limited on 10/26/15.
//
//

#include "catch.hpp"
#include "choreograph/Choreograph.h"

#define TEST_WITH_GLM_VECTORS 0
#if TEST_WITH_GLM_VECTORS

#include "glm/glm.hpp"
#include <array>

using namespace choreograph;
using namespace std;
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
