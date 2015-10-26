//
//  Phrase_test.cpp
//
//  Created by Soso Limited on 10/26/15.
//
//

#include "catch.hpp"
#include "choreograph/Choreograph.h"

using namespace choreograph;
using namespace std;

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
