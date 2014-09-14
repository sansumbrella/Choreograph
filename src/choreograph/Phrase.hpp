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

// Includes needed for tests.
#include <functional>
#include <vector>

namespace choreograph
{
// A motion describes a one-dimensional change through time.
// These can be ease equations, always return the same value, or sample some data. Whatever you want.
// For classic motion, EaseFn( 0 ) = 0, EaseFn( 1 ) = 1.
// set( 0.0f ).move( )
// then again, might just make a curve struct that is callable and have a way to set its initial and final derivatives.
typedef std::function<float (float)> EaseFn;

namespace
{
  /// Default ease function for holds. Keeps constant value output.
  inline float easeHold( float t ) { return 0.0f; }
  /// Default ease function for ramps.
  inline float easeNone( float t ) { return t; }

} // namespace

/// A Position describes a point in time.
template<typename T>
struct Position
{
  Position() = default;

  Position( const T &value, float time = 0.0f ):
    time( time ),
    value( value )
  {}

  float time = 0.0f;
  T     value;
};

/// The default templated lerp function.
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
class Phrase
{
public:
  using LerpFn = std::function<T (const T&, const T&, float)>;

  Phrase( const T &_end, float duration, const EaseFn &easeFn = &easeNone ):
  _end( _end, duration ),
  _easeFn( easeFn )
  {}

  Phrase( const Position<T> &start, const Position<T> &_end, const EaseFn &easeFn = &easeNone ):
  _start( start ),
  _end( _end ),
  _easeFn( easeFn )
  {}

  ~Phrase() = default;

  /// Set start time to \a time. End time is adjusted to preserve duration.
  void shiftStartTimeTo( float time ) {
    float delta = time - _start.time;
    _start.time = time;
    _end.time += delta;
  }

  /// Change start value to \a value.
  void setStartValue( const T &value )
  {
    _start.value = value;
  }

  /// Returns the value at the beginning of this phrase.
  inline const T& getStartValue() const { return _start.value; }
  /// Returns the value at the end of this phrase.
  inline const T& getEndValue() const { return _end.value; }

  /// Returns the time at the beginning of this phrase.
  inline float getStartTime() const { return _start.time; }
  /// Returns the time at the end of this phrase.
  inline float getEndTime() const { return _end.time; }

  /// Returns normalized time if t is in range [_start.time, _end.time].
  inline float normalizeTime( float t ) const { return (t - _start.time) / (_end.time - _start.time); }
  /// Returns the duration of this phrase.
  inline float getDuration() const { return _end.time - _start.time; }

  /// Returns the interpolated value at the given time.
  T getValue( float atTime ) const
  {
    return _lerpFn( _start.value, _end.value, _easeFn( normalizeTime( atTime ) ) );
  }
private:
  Position<T> _start;
  Position<T> _end;
  EaseFn      _easeFn;
  LerpFn      _lerpFn = &lerpT<T>;
};

/**
 Phrase with separately-interpolated components.
 Allows for the use of separate ease functions per component.
 All components must be of the same type.
 */
template<typename T>
class Phrase2
{
public:
  Phrase2( const T &_end, float duration, const EaseFn &ease_x, const EaseFn &ease_y ):
    _end( _end ),
    _ease_x( ease_x ),
    _ease_y( ease_y )
  {}

  Phrase2( const Position<T> &start, const Position<T> &_end, const EaseFn &ease_x, const EaseFn &ease_y ):
    _start( start ),
    _end( _end ),
    _ease_x( ease_x ),
    _ease_y( ease_y )
  {}

  /// Returns the value at the beginning of this phrase.
  inline const T& getStartValue() const { return _start.value; }
  /// Returns the value at the _end of this phrase.
  inline const T& getEndValue() const { return _end.value; }

  /// Returns the time at the beginning of this phrase.
  inline float getStartTime() const { return _start.time; }
  /// Returns the time at the _end of this phrase.
  inline float getEndTime() const { return _end.time; }

  /// Returns normalized time if t is in range [start.time, _end.time].
  inline float normalizeTime( float t ) const { return (t - _start.time) / (_end.time - _start.time); }
  /// Returns the duration of this phrase.
  inline float getDuration() const { return _end.time - _start.time; }

  /// Returns the interpolated value at the given time.
  T getValue( float atTime ) const
  {
    float t = normalizeTime( atTime );
    return T( componentLerpFn( getStartValue().x, getEndValue().x, _ease_x( t ) ),
             componentLerpFn( getStartValue().y, getEndValue().y, _ease_y( t ) ) );
  }

  /// Set ease functions for first and second components.
  void setEase( const EaseFn &component_0, const EaseFn &component_1 ) {
    _ease_x = component_0;
    _ease_y = component_1;
  }

private:
  using ComponentT = decltype( T().x ); // get the type of the x component;
  using ComponentLerpFn = std::function<ComponentT (const ComponentT&, const ComponentT&, float)>;

  ComponentLerpFn componentLerpFn = &lerpT<ComponentT>;
  Position<T>     _start;
  Position<T>     _end;
  EaseFn          _ease_x;
  EaseFn          _ease_y;
};

} // namespace atlantic
