//
//  Motion_test.cpp
//
//  Created by Soso Limited on 10/26/15.
//
//

#include "catch.hpp"
#include "choreograph/Choreograph.h"

using namespace choreograph;
using namespace std;

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
