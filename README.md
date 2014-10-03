# Choreograph

A simple C++11 animation and timing library.  
v0.2.0 development. API is stabilizing.

## Features
- Timeline-based animation of generic properties.
- Composable animation Phrases to mix and loop other Phrases.
- Chainable animation Sequence-building syntax.
- Separable easing of vector components with RampToN Phrases.
- Motion event callbacks (start, update, finish).
- Pseudo-instancing of Sequences to animate multiple targets.
- Support for custom ease functions.
- Support for custom interpolation methods.
- Specialization of default interpolation slerps cinder/glm quaternions.
- Reversible, time-warpable Motions.
- Cues call function after time elapses.
- Unit-tested.
- Works as a Cinder Block.
	- Clone to Cinder/blocks/Choreograph and include in your project with TinderBox.

## Basic Usage
```c++
	using namespace choreograph;
	Timeline timeline;
	Output<float> value_a;
	Output<float> value_b;

	// Define Sequence in-place
	timeline.apply( &value_b )
					.then<RampTo>( 10.0f, 0.3f )
					.then<Hold>( 10.0f, 0.2f )
					.then<RampTo>( 100.0f, 0.5f )
					.finishFn( [] (Motion<float> &m) { cout << "Finished animating value B." << endl; } );

	// Use a pre-defined sequence
	auto sequence = createSequence( 0.0f ); // create a SequenceRef<float> with starting value of 0.0f
	sequence->set( 100.0f )
		.then<RampTo>( 50.0f, 1.0f )
		.then<Hold>( 50.0f, 1.0f )
		.then<RampTo>( 100.0f, 0.5f )
		.set( 50.0f );
	timeline.apply( &value_a, sequence )
					.finishFn( [] { cout << "Finished animating value A." << endl } );
```

## Concepts

### Phrase, Sequence

A Phrase defines a simple change in value over time. The built-in phrase types include various Ramps and Hold. It is simple to add your own Phrases by extending Phrase<T>.

A Sequence is a collection of Phrases. Sequences are the main object you will manipulate to build animations. A method-chaining syntax allows you to build up sophisticated Sequences one Phrase at a time. Sequences can also be added to each other, either by wrapping them in a Phrase or by duplicating the Phrases from another Sequence.

Sequences and Phrases have a duration but no concept of playback or the current time. They interpolate values within their duration and clamp values below zero and past their duration.

### Motion and Output

A Motion connects a Sequence to an Output. Motions have a sense of starting, finishing, and updating, as well as knowing where in time they currently are.

Outputs (Output<T>) wrap a type so that it can communicate with the Motion that is applied to it about its lifetime. If either the Motion or the Output goes out of scope, the animation on that pointer will stop.

```c++
// Outputs can safely be animated by a choreograph::Timeline
choreograph::Output<vec3>	output;
choreograph::Timeline timeline;
timeline.apply( &output ).then<Hold>( 1.0, 1.0 ).then<RampTo>( vec3( 100 ), 3.0 );
timeline.step( 1.0 / 60.0 );

// If you can't wrap your animation target in an Output template for some reason,
// you can still animate it with a choreograph::Timeline, but you need to synchronize
// the lifetime of both objects.
// Instead, consider creating a choreograph::Sequence and assigning values to rawOutput manually.
// That way you aren't passing raw pointers around.
vec3 rawOutput;
Sequence<vec3> sequence( vec3(1.0) ); // create Sequence with initial value.
sequence.then<Hold>( 1.0, 1.0 ).then<RampTo>( vec3( 100 ), 3.0 );
rawOutput = sequence.getValue( animationTime );
```

Motions can also be connected to raw pointers. If you go this route, you need to be very careful about object lifetime and memory management. The Motion will have no way to know if the raw pointer becomes invalid, and the raw pointer won’t know anything about the Motion.

A Connection object handles the actual lifetime management between the Sequence and the Output. Every Motion object composes a Connection to its Output. When the Connection is broken, the Motion is considered invalid and will be discarded by its parent Timeline.

Generally, you will not create any of the above objects directly, but will receive an interface to them by creating a motion with a Timeline.

### Timeline
Timelines manage a collection of Motions. They provide a straightforward interface for connecting Sequences to Outputs and for building up Sequences in place.

When you create a Motion with a Timeline, you receive a MotionOptions object that provides an interface to manipulate the underlying Sequence as well as the Motion.

## Building and running

Include the headers in your search path and add the .cpp files to your project (drag them in) and everything should just work. If you are working with Cinder, you can create a new project with Tinderbox and include Choreograph as a block.

Development, including running tests, is done with the ChoreographDev project. The dev app is a dumping ground for various thoughts on the library. Code likely won’t live there for long, but may find new life in the tests or as a sample. The dev project is set up to work with Xcode 6 on Mac OS 10.9 and Visual Studio 2013 on Windows 7.

### Dependencies

Choreograph itself has no third-party dependencies.

You will need a modern C++ compiler. Choreograph is known to work with Apple LLVM 6.0 (Clang 600), and Visual Studio 2013.

Choreograph’s tests use the Catch framework, which is included in the tests/ directory. 

Choreograph_test has no linker dependencies, but will try to include vector and ease function headers from Cinder if INCLUDE_CINDER_HEADERS is true. This will enable a handful of additional tests.

Benchmarks_test relies on the Cinder library. It uses Cinder’s Timer class to measure performance. It also runs a rough comparison of Choreograph’s performance against ci::Timeline.

Choreograph’s samples and dev application use Cinder for system interaction and graphics display. Any recent version of [Cinder's glNext branch](https://github.com/cinder/cinder/tree/glNext) should work. Clone Choreograph to your blocks directory to have the dev and sample projects work out of the box.

Samples are run from the projects inside the Samples directory. These are more of a work in progress than the rest of the library.

### Using the lerp specializations
If you are using Cinder, the relevant specialization header should be included by Choreograph.hpp automatically.

## History/Tweening alternatives:
Cinder's Timeline is an excellent, production-ready tweening option. It has a stable and proven API. It is based on the previous version of Choreograph.  
Choreograph itself was originally inspired by Flash tweening libraries like Greensock’s TweenMax. While they aren’t for C++, you might draw your own inspiration from them.
