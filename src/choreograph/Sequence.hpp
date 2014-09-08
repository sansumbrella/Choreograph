/*
 * Copyright (c) 2014 David Wicks, sansumbrella.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 #pragma once

namespace choreograph
{

// A motion describes a one-dimensional change through time.
// These can be ease equations, always return the same value, or sample some data. Whatever you want.
// For classic motion, EaseFn( 0 ) = 0, EaseFn( 1 ) = 1.
// set( 0.0f ).move( )
// then again, might just make a curve struct that is callable and have a way to set its initial and final derivatives.
typedef std::function<float (float)> EaseFn;

struct Hold
{
  float operator() ( float t ) const { return 0.0f; }
};

struct LinearRamp
{
  float operator() ( float t ) const { return t; }
};

//! A Position describes a point in time.
template<typename T>
struct Position
{
  T     value;
  float time;
};

//! The default templated lerp function.
template<typename T>
T lerpT( const T &a, const T &b, float t )
{
  return a + (b - a) * t;
}

/**
 A Phrase is a part of a Sequence.
 It describes the motion between two positions.
 This is the essence of a Tween, with all values held internally.
 */
template<typename T>
struct Phrase
{
  Position<T> start;
  Position<T> end;
  EaseFn      motion;
  std::function<T (const T&, const T&, float)> lerpFn = &lerpT<T>;

  T getValue( float atTime ) const
  {
    float t = (atTime - start.time) / (end.time - start.time);
    return lerpFn( start.value, end.value, motion( t ) );
  }
};

/**
 Phrase with separately-interpolated components.
 Allows for the use of separate ease functions per component.
 */
template<typename T>
struct Phrase2
{
  Position<T> start;
  Position<T> end;
  using ComponentT = decltype( std::declval<T>().x );
  std::function<ComponentT (const ComponentT&, const ComponentT&, float)> lerpFn = &lerpT<ComponentT>;
  EaseFn      motion1;
  EaseFn      motion2;

  T getValue( float atTime ) const
  {
    float t = (atTime - start.time) / (end.time - start.time);
    return T( lerpFn( start.value.x, end.value.x, motion1( t ) ), lerpFn( start.value.y, end.value.y, motion2( t ) ) );
  }
};

/**
 A Sequence of motions.
 Our essential compositional tool, describing all the transformations to one element.
 A kind of platonic idea of an animation sequence; this describes a motion without giving it an output.
*/
template<typename T>
class Sequence
{
public:
  Sequence() = delete;

  //! Construct a Sequence with \a value initial value.
  explicit Sequence( const T &value ):
    _initial_value( value )
  {}

  T getValue( float atTime );

  //! Set current value. An instantaneous hold.
  Sequence<T>& set( const T &value )
  {
    if( _segments.empty() ) {
      _initial_value = value;
    }
    else {
      hold( value, 0.0f );
    }
    return *this;
  }

  //! Returns a copy of this sequence. Useful if you want to make a base animation and modify that.
  std::shared_ptr<Sequence<T>> copy() const { return std::make_shared<Sequence<T>>( *this ); }

  //! Hold on current end value for \a duration seconds.
  Sequence<T>& wait( float duration ) { return hold( duration ); }

  //! Hold on current end value for \a duration seconds.
  Sequence<T>& hold( float duration )
  {
    return hold( endValue(), duration );
  }

  //! Hold on \a value for \a duration seconds.
  Sequence<T>& hold( const T &value, float duration )
  {
    Phrase<T> s;
    s.start = Position<T>{ value, _duration };
    s.end = Position<T>{ value, _duration + duration };
    s.motion = Hold();

    _segments.push_back( s );

    _duration += duration;

    return *this;
  }

  //! Animate to \a value over \a duration seconds using \a ease easing.
  Sequence<T>& rampTo( const T &value, float duration, const EaseFn &ease = LinearRamp() )
  {
    Phrase<T> s;
    s.start = Position<T>{ endValue(), _duration };
    s.end = Position<T>{ value, _duration + duration };
    s.motion = ease;

    _segments.push_back( s );

    _duration += duration;

    return *this;
  }

  //! Sets the ease function of the last Phrase in the Sequence.
  Sequence<T>& ease( const EaseFn &easeFn ) {
    if( ! _segments.empty() ) {
      _segments.back().motion = easeFn;
    }
    return *this;
  }

  //! Returns the number of seconds required to move through all Phrases.
  float getDuration() const { return _duration; }

  //! Returns the value at the end of the Sequence.
  T endValue() const { return _segments.empty() ? _initial_value : _segments.back().end.value; }

  //! Returns the value at the beginning of the Sequence.
  T initialValue() const { return _initial_value; }

private:
  std::vector<Phrase<T>>  _segments;
  T                       _initial_value;
  float                   _duration = 0.0f;

  friend class Timeline;
};

//! Returns the value of this sequence for a given point in time.
// Would be nice to have a constant-time check (without a while loop).
template<typename T>
T Sequence<T>::getValue( float atTime )
{
  if( atTime < 0.0f )
  {
    return _initial_value;
  }
  else if ( atTime >= _duration )
  {
    return endValue();
  }

  auto iter = _segments.begin();
  while( iter < _segments.end() ) {
    if( (*iter).end.time > atTime )
    {
      return (*iter).getValue( atTime );
    }
    ++iter;
  }
  // past the end, get the final value
  // this should be unreachable, given that we return early if time >= duration
  return endValue();
}

template<typename T>
using SequenceRef = std::shared_ptr<Sequence<T>>;

} // namespace choreograph
