# Choreograph

A simple (header-only?) C++11 animation and timing library.

## Features
- Timeline-based animation of generic properties.
- Use same Sequence to animate multiple properties.
- Chainable animation properties.
- Motion event callbacks (start, update, finish).
- Cues (function callback at a given time).
- Support for custom ease functions.
- Support for custom interpolation methods.

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

## Custom types
```c++

```

Tweening alternatives:
Choreograph v1 eventually became cinder::Timeline.
