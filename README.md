# Choreograph

A simple C++11 animation and timing library.  
v0.2.0 development. API is unstable.

## Features
- Timeline-based animation of generic properties.
- Use same Sequence to animate multiple properties.
- Chainable animation syntax.
- Motion event callbacks (start, update, finish).
- Delayed/staggered animation callbacks.
- Support for custom ease functions.
- Support for custom interpolation methods.
- TODO: Cues (function callback at a given time).
- Template specializations support intuitive animation of glm quaternions.
- Separable easing of vector components.
- Reversible, time-warpable motions.

## Basic Usage
```c++
	using namespace choreograph;
	Timeline timeline;

	// Use a pre-defined sequence
	auto sequence = make_shared<Sequence<float>>();
	sequence->set( 100.0f )
		.rampTo( 50.0f, 1.0f )
		.hold( 1.0f )
		.rampTo( 100.0f, 0.5f )
		.set( 50.0f );

	float some_value;

	timeline.move( &some_value, sequence ).finishFn( [] { cout << "Finished animating some value" << endl; });

	// Create a sequence in-place.
	vec2 other_value;
	timeline.move( &other_value ).getSequence().rampTo( vec2( 100.0f, 500.0f ), 0.33f );
```

## Concepts

### Position, Phrase, Sequence
A Position is a value at a point in time.

A Phrase connects two Positions with a transition. Phrases are the basic atom of choreograph.

A Sequence is a collection of phrases. Sequences are the main object you will manipulate to build animations. A method-chaining syntax allows you to build up sophisticated Sequences one Phrase at a time.

Sequences are timeless, you can query their value at any point in time and always get a valid response; they are clamped at their endpoints.

### Motion and Output
A Motion is the application of a choreograph Sequence. It uses a Sequence of Phrases to move some external variable in time. Motions have a sense of starting, finishing, and updating, as well as knowing where in time they should happen and currently are.

Motions apply the values from a Sequence to an Output. Outputs (Output<T>) wrap a type so that it can communicate with the Motion that is applied to it about its lifetime. If either the Motion or the Output goes out of scope, the animation on that pointer will stop. You can use a raw pointer to any type as an output, as well, but you need to be very careful about object lifetime and memory management if you do.

### Timeline
Timelines manage a collection of Motions.

### Using the lerp specializations
If you are using Cinder, the relevant specialization header should be included by Choreograph.hpp automatically.

#### History/Tweening alternatives:
Cinder's Timeline is an excellent, production-ready tweening option. It is based on the previous version of Choreograph.  
Choreograph itself was originally inspired by Flash tweening libraries like Greensock's TweenMax.
