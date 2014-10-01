# Choreograph

A simple C++11 animation and timing library.  
v0.2.0 development. API is unstable.

## Features
- Timeline-based animation of generic properties.
- Pseudo-instancing of Sequences to animate multiple targets.
- Chainable animation syntax.
- Motion event callbacks (start, update, finish).
- Support for custom ease functions.
- Support for custom interpolation methods.
- Separable easing of vector components.
- Template specializations support intuitive animation of cinder/glm quaternions.
- Reversible, time-warpable motions.
- Cues (function callback at a given time).
- Works as a Cinder Block.
	- Clone to Cinder/blocks/Choreograph and include in your project with TinderBox.

## Basic Usage
```c++
	using namespace choreograph;
	Timeline timeline;
	Output<float> value_a;
	Output<float> value_b;

	// Use a pre-defined sequence
	auto sequence = make_shared<Sequence<float>>();
	sequence->set( 100.0f )
		.then<RampTo>( 50.0f, 1.0f )
		.then<Hold>( 50.0f, 1.0f )
		.then<RampTo>( 100.0f, 0.5f )
		.set( 50.0f );
	timeline.apply( &value_a, sequence )
					.finishFn( [] { cout << "Finished animating value A." << endl } );

	// Define Sequence in-place
	timeline.apply( &value_b )
					.then<RampTo>( 10.0f, 0.3f )
					.then<Hold>( 10.0f, 0.2f )
					.then<RampTo>( 100.0f, 0.5f )
					.finishFn( [] { cout << "Finished animating value B." << endl; } );
```

## Concepts

### Phrase, Sequence

A Phrase defines a simple change in value over time. The built-in phrase types include various Ramps and Holds. It is simple to add your own Phrases by extending Phrase<T>.

A Sequence is a collection of phrases. Sequences are the main object you will manipulate to build animations. A method-chaining syntax allows you to build up sophisticated Sequences one Phrase at a time. Sequences can also be added to each other, either by wrapping them in a Phrase or by duplicating the Phrases from another Sequence.

Sequences and Phrases have a duration but no concept of what time it is or playing. They interpolate values within their duration and clamp values below zero and past their duration.

### Motion and Output

A Motion connects a Sequence to an Output. Motions have a sense of starting, finishing, and updating, as well as knowing where in time they currently are.

Outputs (Output<T>) wrap a type so that it can communicate with the Motion that is applied to it about its lifetime. If either the Motion or the Output goes out of scope, the animation on that pointer will stop.

Motions can also be connected to raw pointers. If you go this route, you need to be very careful about object lifetime and memory management. The Motion will have no way to know if the raw pointer becomes invalid, and the raw pointer won’t know anything about the Motion.

A Connection object handles the actual lifetime management between the Sequence and the Output. It is composed into the Motion, so you don’t need to worry about its details.

Generally, you will not create any of the above objects directly, but will receive an interface to them by creating a motion with a Timeline.

### Timeline
Timelines manage a collection of Motions. They provide a straightforward interface for connecting Sequences to Outputs and for building up Sequences in place.

When you create a Motion with a Timeline, you receive a MotionOptions object that can manipulate the underlying Sequence as well as the Motion.

## Building and running

Initial v0.2.0 development is being done on OSX with Xcode 6. Once I have a relatively stable API, I will make sure things are building and happy in Visual Studio 2013.

Development, including running tests, is done with the ChoreographDev project. The dev app is a dumping ground for various thoughts on the library. Code likely won’t live there for long, but may find new life in the tests or as a sample.

### Dependencies

Choreograph itself has no third-party dependencies.

You will need a modern C++ compiler. I am developing Choreograph against Apple LLVM 6.0 (Clang 600), and will eventually ensure compatibility with Visual Studio 2013.

Choreograph’s tests use the Catch framework, which is included in the tests/ directory. 

Choreograph_test has no linker dependencies, but will try to include vector and ease function headers from Cinder if INCLUDE_CINDER_HEADERS is true. This will enable a handful of additional tests.

Benchmarks_test relies on the Cinder library. It uses Cinder’s Timer class to measure performance. It also runs a rough comparison of Choreograph’s performance against ci::Timeline.

Choreograph’s samples and dev application use Cinder for system interaction and graphics display. Any recent version of [Cinder's glNext branch](https://github.com/cinder/cinder/tree/glNext) should work. Clone Choreograph to your blocks directory to have the dev and sample projects work out of the box.

Samples are run from the projects inside the Samples directory. These are a work in progress.

### Using the lerp specializations
If you are using Cinder, the relevant specialization header should be included by Choreograph.hpp automatically.

## History/Tweening alternatives:
Cinder's Timeline is an excellent, production-ready tweening option. It is based on the previous version of Choreograph.  
Choreograph itself was originally inspired by Flash tweening libraries like Greensock's TweenMax. While they aren't for C++, you might draw your own inspiration from them.
