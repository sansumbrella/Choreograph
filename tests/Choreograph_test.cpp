// Some day, I will figure out how to use Catch nicely within Xcode to run some tests

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "choreograph/Choreograph.hpp"
#include "cinder/Timer.h"

using namespace std;
using namespace cinder;
using namespace choreograph;

class ScopedTimer
{
public:
  ScopedTimer( const std::string &message ):
    _message( message )
  {}
  ~ScopedTimer()
  {
    _timer.stop();
    cout << "[" + _message + "] Elapsed time: " << _timer.getSeconds() * 1000 << "ms" << endl;
  }
private:
  std::string _message;
  ci::Timer   _timer = ci::Timer( true );
};

TEST_CASE( "Sequences can be whatevered", "[sequence]" )
{

}

TEST_CASE( "Sequence", "[sequence]" ) {
  Sequence<float> sequence( 0.0f );
  // 4 second sequence.
  sequence.set( 1.0f ).hold( 1.0f ).rampTo( 2.0f, 1.0f ).rampTo( 10.0f, 1.0f ).rampTo( 2.0f, 1.0f );

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
    REQUIRE( sequence.getValueWrapped( sequence.getDuration() + offset ) == sequence.getValueWrapped( offset ) );
    REQUIRE( sequence.getValueWrapped( (7 * sequence.getDuration()) + offset ) == sequence.getValueWrapped( offset ) );
  }
}
