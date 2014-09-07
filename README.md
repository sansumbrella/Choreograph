# Choreograph

A simple C++11 animation and timing library. [WIP]

## Features
- Timeline-based animation of generic properties.
- Use same Sequence to animate multiple properties.
- Chainable animation syntax.
- Motion event callbacks (start, update, finish).
- Cues (function callback at a given time).
- Delayed/staggered animation callbacks.
- Support for custom ease functions.
- Support for custom interpolation methods.
- Specializations support intuitive animation of Cinder and GLM quaternions.

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

### Motion
A Motion is the application of a choreograph Sequence. It uses a Sequence of Phrases to move some external variable in time. Motions have a sense of starting, finishing, and updating, as well as knowing where in time they should happen and currently are.

### Timeline
Timelines manage a collection of Motions.

## Animating Custom types
```c++

```

### Using the lerp specializations
Add the relevant specialization cpp file to your build sources.

Tweening alternatives:
Choreograph v1 eventually became cinder::Timeline.
