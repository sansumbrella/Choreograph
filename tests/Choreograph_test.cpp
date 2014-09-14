// Some day, I will figure out how to use Catch nicely within Xcode to run some tests

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "choreograph/Choreograph.hpp"
#include "cinder/Timer.h"
#include "cinder/Vector.h"
#include "cinder/Timeline.h"

using namespace std;
using namespace cinder;
using namespace choreograph;

class ScopedTimer
{
public:
  ScopedTimer( const std::string &message, float *output = nullptr ):
    _message( message ),
    _output( output )
  {}
  ~ScopedTimer()
  {
    _timer.stop();
    cout << "[" + _message + "] Elapsed time: " << _timer.getSeconds() * 1000 << "ms" << endl;
    if( _output ) {
      *_output = _timer.getSeconds() * 1000;
    }
  }
private:
  float       *_output = nullptr;
  std::string _message;
  ci::Timer   _timer = ci::Timer( true );
};


TEST_CASE( "Separate component interpolation", "[sequence]" ) {
  Sequence<vec2, Phrase2<vec2>> sequence( vec2( 1 ) );
  sequence.then( vec2( 10.0f ), 1.0f, EaseOutQuad(), EaseInQuad() );
  for( float t = 0.0f; t <= 1.01f; t += 0.1f ) {
    cout << "T: " << t << ", value: " << sequence.getValue( t ) << endl;
  }
}

TEST_CASE( "Sequence Interpolation", "[sequence]" ) {
  const float epsilon = std::numeric_limits<float>::epsilon() * 20;

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

    REQUIRE( (sequence.getTimeWrapped( 10 * sequence.getDuration() + offset ) - offset) < epsilon );
    REQUIRE( (sequence.getValueWrapped( sequence.getDuration() + offset ) - sequence.getValue( offset ) ) < epsilon );
    REQUIRE( (sequence.getValueWrapped( (2 * sequence.getDuration()) + offset ) - sequence.getValue( offset ) ) < epsilon );
    REQUIRE( (sequence.getValueWrapped( (50 * sequence.getDuration()) + offset ) - sequence.getValue( offset ) ) < epsilon );
  }
}

TEST_CASE( "Output Connections", "[output]" ) {

  co::Timeline timeline;
  auto sequence = make_shared<Sequence<float>>( 0.0f );
  sequence->rampTo( 10.0f, 2.0f );

  SECTION( "Output falling out of scope disconnects" ) {
    { // create locally scoped output
      Output<vec4> temp;
      timeline.move( &temp ).getSequence().rampTo( vec4( 5 ), 1.0f );

      REQUIRE( timeline.size() == 1 );
    }

    // Part of test is that nothing fails when stepping the timeline.
    timeline.step( 0.5f );
    REQUIRE( timeline.empty() == true );
  }

  SECTION( "Motion falling out of scope disconnects" ) {
    Output<float> output;

    { // hook up a motion to our output
      Motion<float> temp( &output, sequence );

      REQUIRE( output.isConnected() == true );
    }

    REQUIRE( output.isConnected() == false );
  }

  SECTION( "Vector of outputs can be moved." ) {
    vector<Output<float>> outputs( 500, 0.0f );
    vector<Output<float>> copy;

    for( auto &output : outputs ) {
      timeline.move( &output, sequence );
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
    motion.skipTo( 1.0f );

    REQUIRE( copy.value() == 5.0f );

    motion.skipTo( 2.0f );
    REQUIRE( copy.value() == 10.0f );
  }

  SECTION( "Vector of outputs can be copied." ) {
    vector<Output<float>> outputs( 500, 0.0f );
    vector<Output<float>> copy;

    for( auto &output : outputs ) {
      timeline.move( &output, sequence );
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
    motion.skipTo( 1.0f );

    REQUIRE( base.value() == 1.0f );
    REQUIRE( copy.value() == 5.0f );

    motion.skipTo( 2.0f );
    REQUIRE( copy.value() == 10.0f );
  }
}

TEST_CASE( "Performance stuff", "[library]" ) {
  co::Timeline test_timeline;
  ci::TimelineRef cinder_timeline = ci::Timeline::create();

  const int tween_count = 5000;
  const float dt = 1.0f / 60.0f;

  SECTION( "Tween many targets" ) {
    for( int j = 0; j < 2; ++j )
    {
      vector<Output<vec2>> targets( tween_count, vec2( 0 ) );
      vector<Anim<vec2>> cinder_targets( tween_count, vec2( 0 ) );
      float choreograph, cinder;

      { // Measure basic performance.
        ScopedTimer performance( "Choreograph Timeline", &choreograph );
        //
        for( int i = 0; i < tween_count; ++i )
        {
          test_timeline.move( &targets[i] ).getSequence().hold( 1.0f ).rampTo( vec2( i * 5, i * 20 ), 2.0f );
        }
        for( float t = 0.0f; t <= 3.0f; t += dt ) {
          test_timeline.step( dt );
        }
      }

      { // Compare to cinder timeline perf.
        ScopedTimer perf( "Cinder Timeline", &cinder );
        for( int i = 0; i < tween_count; ++i )
        {
          cinder_timeline->apply( &cinder_targets[i], vec2( i * 5, i * 20 ), 2.0f ).delay( 1.0f );
        }
        for( float t = 0.0f; t <= 3.0f; t += dt ) {
          cinder_timeline->step( dt );
        }
      }

      cout << "Comparative timeline performance (Choreograph:Cinder): " << (choreograph / cinder) << endl;
    }

  }

  SECTION( "Tween one target many times" ) {
    for( int j = 0; j < 2; ++j )
    {
      Output<vec2>  target( vec2( 0 ) );
      Anim<vec2>    cinder_target( vec2( 0 ) );
      float choreograph, cinder;

      { // Measure basic performance.
        ScopedTimer performance( "Choreograph Timeline", &choreograph );
        //
        for( int i = 0; i < tween_count; ++i )
        {
          test_timeline.queue( &target ).hold( 1.0f ).rampTo( vec2( i * 5, i * 20 ), 2.0f );
        }
        for( float t = 0.0f; t <= 3.0f; t += dt ) {
          test_timeline.step( dt );
        }
      }

      { // Compare to cinder timeline perf.
        ScopedTimer perf( "Cinder Timeline", &cinder );
        for( int i = 0; i < tween_count; ++i )
        {
          cinder_timeline->appendTo( &cinder_target, vec2( i * 5, i * 20 ), 2.0f ).delay( 1.0f );
        }
        for( float t = 0.0f; t <= 3.0f; t += dt ) {
          cinder_timeline->step( dt );
        }
      }

      cout << "Comparative timeline performance (Choreograph:Cinder): " << (choreograph / cinder) << endl;
    }
  }
  cout << endl << endl;
}
