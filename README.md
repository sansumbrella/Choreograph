# Choreograph

A simple C++11 animation and timing library.  
v0.2.0 development. API is stabilizing.

Choreograph is designed to describe motion. With it, you compose motion Phrases into Sequences that can be used to animate arbitrary properties on a Timeline.

## Basic Usage
```c++
  using namespace choreograph;
  Timeline timeline;
  Output<float> target = 10.0f;

  // Build a Sequence from a series of Phrases.
  Sequence<float> continuation( 0.0f );
  continuation.then<RampTo>( 100.0f, 1.0f )
              .then<Hold>( 50.0f, 0.5f );

  // Create a Sequence in-place on the Timeline.
  timeline.apply( &target )
          // Animate target value to 50 over .3 seconds.
          .then<RampTo>( 50.0f, 0.3f )
          // Stay at 50 for .2 more seconds.
          .then<Hold>( 50.0f, 0.2f )
          // Repeat the motion in our previously created Sequence three times.
          .then( makeRepeat( continuation.asPhrase(), 3 ) )
          // When finished playing, print out some info.
          .finishFn( [] (Motion<float> &m) { cout << "Finished animating target." << endl; } );

  timeline.step( 1.0 / 60.0 );
```

Read the concepts section below and see the projects in Samples/ for more ideas on how to use Choreograph in your own projects.

## Concepts

### Phrase, Sequence

A Phrase defines the behavior of a value over time. The built-in phrase RampTo linearly interpolates a value over time. Other built-in Phrases include Hold, which holds a value for an amount of time, and LoopPhrase, which takes an existing Phrase and repeats it a number of times. It is simple to add your own Phrases by extending `Phrase<T>`.

A Sequence is a collection of Phrases. Sequences are the primary object you manipulate to build animations. Choreograph’s method-chaining syntax allows you to build up sophisticated Sequences by telling them to do one Phrase `then` another. Sequences can compose other Sequences, too, by either wrapping the Sequence in a Phrase or duplicating the other’s Phrases.

Sequences and Phrases have duration but no concept of playback or the current time. They interpolate values within their duration and clamp values before zero and after their duration.

### Motion and Output

A Motion connects a Sequence to an Output. Motions have a sense of starting, finishing, and updating, as well as knowing where in time they currently are.

Outputs are values that can safely be animated with a Motion. The `Output<T>` template wraps any type so that it can communicate with the Motion applied to it about its lifetime. If either the Motion or the Output goes out of scope, the animation on that Output will stop.

Motions can also be connected to raw pointers using the `Timeline::applyRaw()` family of methods. If you go this route, you need to be very careful about object lifetime and memory management. The Motion has no way to know if the raw pointer becomes invalid, and the raw pointer won’t know anything about the Motion.

A Connection object handles the lifetime management between the Motion and the Output. Every Motion object composes a Connection to its Output. When the Connection is broken, the Motion is considered invalid and will be discarded by its parent Timeline.

Generally, you will not create Motions or Connections directly, but will receive an interface to them by creating a Motion with a Timeline.

```c++
// Outputs can safely be animated by a choreograph::Timeline
choreograph::Output<vec3> target;
choreograph::Timeline timeline;
// Create a Motion with a Connection to target and modify
// the Motion’s underlying Sequence.
timeline.apply( &target ).then<Hold>( vec3( 1.0 ), 1.0 )
                        .then<RampTo>( vec3( 100 ), 3.0 );
timeline.step( 1.0 / 60.0 );
```

If you cannot wrap your animation target in an Output template, consider creating a Sequence and assigning its value to your target manually. That way you aren’t storing raw pointers in a timeline.

```c++
// Recommended approach to animating non-Output types
vec3 target;
// Create a Sequence with initial value from target.
Sequence<vec3> sequence( target );
sequence.then<Hold>( 1.0, 1.0 ).then<RampTo>( vec3( 100 ), 3.0 );
target = sequence.getValue( animationTime );
```

### Timeline
Timelines manage a collection of TimelineItems (Motions, Cues, &c). They provide a straightforward interface for connecting Sequences to Outputs and for building up Sequences in-place.

When you create a Motion with a Timeline, you receive a MotionOptions object that provides an interface to manipulate the underlying Sequence as well as the Motion.

Since Motions know where they are in time, the Timeline controlling them doesn’t need to. While that may seem strange, this allows us to keep timelines running in perpetuity without worrying about running out of numerical precision. The largest number we ever need to keep precise is the duration of our longest Sequence and its corresponding Motion. Also, I find it feels natural that new Motions always start at time zero.

## Building and running

Include the headers in your search path and add the .cpp files to your project (drag them in) and everything should just work. If you are working with Cinder, you can create a new project with Tinderbox and include Choreograph as a block.

### Dependencies

Choreograph itself has no third-party dependencies.

You do need a modern C++ compiler. Choreograph is known to work with Apple LLVM 6.0 (Clang 600), and Visual Studio 2013.

### Building the Tests

Tests are built and run with the projects inside the tests/ directory. There are test projects for Xcode 6 and Visual Studio 2013. Choreograph’s tests use the [Catch](https://github.com/philsquared/Catch) framework, a single-header library that is included in the tests/ directory.

Choreograph_test has no linker dependencies, but will try to include vector and ease function headers from Cinder if INCLUDE_CINDER_HEADERS is true. Including the Cinder headers enables a handful of additional tests, notably those covering the separable component easing of RampToN.

Benchmarks_test relies on the Cinder library. It uses Cinder’s Timer class to measure performance. Benchmarks_test also runs a rough performance comparison between choreograph::Timeline and cinder::Timeline.

### Building the Samples

Choreograph’s samples use Cinder for system interaction and graphics display. Any recent version of [Cinder's glNext branch](https://github.com/cinder/cinder/tree/glNext) should work. Clone Choreograph to your blocks directory to have the sample project work out of the box.

Samples are run from the projects inside the Samples directory. Projects to build the samples exist for iOS and OSX using Xcode and for Windows Desktop using Visual Studio 2013. These are more of a work in progress than the rest of the library.

### Lerp Specializations
If you are using Cinder, Choreograph will include a specialization header to slerp quaternions.

## History/Tweening alternatives:
Cinder's Timeline is an excellent, production-ready tweening option. It has a stable and proven API. It is based on the previous version of Choreograph.

Choreograph itself was originally inspired by Flash tweening libraries like Greensock’s TweenMax. While they aren’t for C++, you might draw your own inspiration from them.
