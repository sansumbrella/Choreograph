# Choreograph

A simple, modern C++ animation and timing library.  
v0.4.0

Choreograph is designed to describe motion. With it, you compose motion Phrases into Sequences that can be used to animate arbitrary properties on a Timeline.

## Basic Usage

Simple things are simple. You can animate a variable through a sequence of values in Choreograph by creating a sequence of ramps.

```c++
// Create a Sequence that is applied to a variable.
timeline.apply( &variable )
  .then<RampTo>( value, 1.0f, EaseInOutQuad() )
  .then<RampTo>( other_value, 0.5f );
```

Choreograph also provides a range of more sophisticated phrases, including procedures and accumulators. These allow you to do things like combine a left-to-right ramp with a sine-wave procedure. The code shown below is elaborated in the Oscillator sample.

```c++
  // Create a procedural phrase that generates a vertical sine wave.
  auto bounce = makeProcedure<vec2>( 2.0, [] ( Time t, Time duration ) {
    return vec2( 0, sin( easeInOutQuad(t) * 6 * M_PI ) * 100.0f );
  } );

  // Create a ramp phrase that moves from left-to-right.
  auto slide = makeRamp( vec2( x1, 0 ), vec2( x2, 0 ), 2.0f, EaseInOutCubic() );

  // Combine the slide and bounce phrases using an AccumulatePhrase.
  float center_y = app::getWindowHeight() / 2.0f;
  auto bounceAndSlide = makeAccumulator<vec2>( vec2( 0, center_y ), bounce, slide );

  timeline.apply( &output, bounceAndSlide );
```

The code above produces the motion of the purple circle below:

![Oscillator Sample](https://cloud.githubusercontent.com/assets/81553/4703448/2a5214ac-586b-11e4-9db5-7b081b4011c3.gif)

Read the concepts section below and see the projects in Samples/ for more ideas on how to use Choreograph in your own projects.

## Concepts

Choreograph breaks the task of animation into two parts: motion description and motion application. Description of motion is handled by `Phrases` and `Sequences`. The application of motion is handled by `Timelines` that manage `TimelineItems`: `Motions` (`Sequences` bound to an `Output`) and `Cues` (Functions bound to a point in time).

### Phrase, Sequence

A Phrase defines a change in value over time, like "bounce up and down for three seconds."" The built-in phrase RampTo linearly interpolates a value over time. Other built-in Phrases include Hold, which holds a value for an amount of time, and LoopPhrase, which takes an existing Phrase and repeats it a number of times. It is simple to add your own Phrases by extending `Phrase<T>`.

A Sequence is a collection of Phrases. Sequences are the primary object you manipulate to build animations. Choreograph’s method-chaining syntax allows you to build up sophisticated Sequences by telling them to do one Phrase `then` another. Sequences can compose other Sequences, too, by either wrapping the Sequence in a Phrase or duplicating the other’s Phrases.

Sequences and Phrases have duration but no concept of playback or the current time. They interpolate values within their duration and clamp values before zero and after their duration.

![diagram-phrase-sequence](https://cloud.githubusercontent.com/assets/81553/4703002/31e31d32-5867-11e4-8296-f6887338dd99.jpg)

### Motion and Output

A Motion connects a Sequence to an Output. Motions have a sense of starting, finishing, and updating, as well as knowing where in time they currently are.

Outputs are values that can safely be animated with a Motion. The `Output<T>` template wraps any type so that it can communicate with the Motion applied to it about its lifetime. If either the Motion or the Output goes out of scope, the animation on that Output will stop.

![diagram-motion](https://cloud.githubusercontent.com/assets/81553/4703117/2268d490-5868-11e4-8435-789f83e07eee.jpg)

Generally, you will not create Motions directly, but will receive an interface to them from a Timeline.

```c++
// Outputs can safely be animated by a choreograph::Timeline
choreograph::Output<vec3> target;
choreograph::Timeline timeline;
// Create a Motion with a Connection to target and modify
// the Motion’s underlying Sequence.
timeline.apply( &target )
  .then<Hold>( vec3( 1.0 ), 1.0 )
  .then<RampTo>( vec3( 100 ), 3.0 );
timeline.step( 1.0 / 60.0 );
```

Motions can also be connected to raw pointers using the `Timeline::applyRaw()` family of methods. If you go this route, you need to be very careful about object lifetime and memory management. The Motion has no way to know if the raw pointer becomes invalid, and the raw pointer won’t know anything about the Motion.

If you cannot wrap your animation target in an Output template, consider creating a Sequence and assigning its value to your target manually. That way you aren’t storing raw pointers in a timeline.

```c++
// Recommended approach to animating non-Output types
vec3 target;
// Create a Sequence with initial value from target.
auto sequence = Sequence<vec3>( target.value() );
  .then<Hold>( 1.0, 1.0 )
  .then<RampTo>( vec3( 100 ), 3.0 );
target = sequence.getValue( animationTime );
```

### Cues

Cues are functions that are called at a certain point in time. You can add them to a Timeline to trigger events in the future. They are useful for changing application state that isn’t strictly animatable. The sample application uses cues to manage transitions between each of the samples.

You can use a Control to manage the lifetime of your Cue. Controls are also available for Motions, and can be used to manage Motions affecting raw pointers.
```c++
// Controls let you cancel the Cue.
auto control = timeline.cue( [] { "do something..."; }, 10.0f ).getControl();
// Stop the cue from firing by calling cancel().
control.cancel();

// Scoped controls cancel timeline items when they fall out of scope.
// They are handy to store at the same scope as any variables captured by reference in your lambda.
auto scoped_control = timeline.cue( [] { "do something else..."; }, 10.0f ).getScopedControl();
```

### Timeline
Timelines manage a collection of TimelineItems (Motions, Cues, &c). They provide a straightforward interface for connecting Sequences to Outputs and for building up Sequences in-place.

When you create a Motion with a Timeline, you receive a MotionOptions object that provides an interface to manipulate the underlying Sequence as well as the Motion.

![diagram-timeline](https://cloud.githubusercontent.com/assets/81553/4703003/31f3155c-5867-11e4-932d-5e8cd5da1c33.jpg)

Since Motions know where they are in time, the Timeline controlling them doesn’t need to. While that may seem strange, this allows us to keep timelines running in perpetuity without worrying about running out of numerical precision. The largest number we ever need to keep precise is the duration of our longest Sequence and its corresponding Motion. Also, I find it feels natural that new Motions always start at time zero.

## Building and running

Include the headers in your search path and add the .cpp files to your project (drag them in) and everything should just work. If you are working with Cinder, you can create a new project with Tinderbox and include Choreograph as a block.

### Dependencies

Choreograph itself has no third-party dependencies.

You do need a modern C++ compiler, since Choreograph takes advantage of a number of C++11 features. Choreograph is known to work with Apple LLVM 6.0 (Clang 600), and Visual Studio 2013.

### Building the Tests

Tests are built and run with the projects inside the tests/ directory. There are test projects for Xcode 6 and Visual Studio 2013. Choreograph’s tests use the [Catch](https://github.com/philsquared/Catch) framework, a single-header library that is included in the tests/ directory.

Choreograph_test has no linker dependencies, but will try to include vector types from Cinder if INCLUDE_CINDER_HEADERS is true. Including the Cinder headers enables a handful of additional tests, notably those covering the separable component easing of RampToN.

Benchmarks_test relies on the Cinder library. It uses Cinder’s Timer class to measure performance. Benchmarks_test also runs a rough performance comparison between choreograph::Timeline and cinder::Timeline.

### Building the Samples

Choreograph’s samples use Cinder for system interaction and graphics display. Any recent version of [Cinder's glNext branch](https://github.com/cinder/cinder/tree/glNext) should work. Clone Choreograph to your blocks directory to have the sample project work out of the box.

Samples are run from the projects inside the Samples directory. Projects to build the samples exist for iOS and OSX using Xcode and for Windows Desktop using Visual Studio 2013. These are more of a work in progress than the rest of the library.

### Interpolating Special Types
If you are using Cinder, Choreograph will automatically include a specialization header so quaternions will be slerped. Where relevant, Phrases also accept an optional interpolation function parameter so you can customize them for other types.

## History

The original motivation for having a library like Choreograph came from past experience with Flash tweening libraries like Greensock’s TweenMax. They made programmatic animations art-directable, which was a huge boon for doing work on a team.

The first version of Choreograph served as a proof-of-concept for what eventually became Timeline in libCinder. Cinder’s timeline is an excellent, production-ready tweening option.

The current iteration of Choreograph is redesigned from whole cloth. The library concepts, outlined above, allow for a smaller, more expressive API than what is possible with a tween-centric system. As a bonus, Choreograph’s new design also outperforms Cinder’s timeline in the included benchmarks for creating and running animations.
