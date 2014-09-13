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
#include <cmath>

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
//! Default ease function for holds. Keeps constant value output.
inline float easeHold( float t ) { return 0.0f; }
//! Default ease function for ramps.
inline float easeNone( float t ) { return t; }

} // namespace

//! A Position describes a point in time.
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
class Phrase
{
public:
  using LerpFn = std::function<T (const T&, const T&, float)>;

  Phrase( const T &end, float duration, const EaseFn &easeFn = &easeNone ):
    end( end, duration ),
    motion( easeFn )
  {}

  Phrase( const Position<T> &start, const Position<T> &end, const EaseFn &easeFn = &easeNone ):
    start( start ),
    end( end ),
    motion( easeFn )
  {}

  virtual ~Phrase() = default;

  //! Set start time to \a time. End time is adjusted to preserve duration.
  void shiftStartTimeTo( float time ) {
    float delta = time - start.time;
    start.time = time;
    end.time += delta;
  }

  //! Change start value to \a value.
  void setStartValue( const T &value )
  {
    start.value = value;
  }

  //! Returns the value at the beginning of this phrase.
  inline const T& getStartValue() const { return start.value; }
  //! Returns the value at the end of this phrase.
  inline const T& getEndValue() const { return end.value; }

  //! Returns the time at the beginning of this phrase.
  inline float getStartTime() const { return start.time; }
  //! Returns the time at the end of this phrase.
  inline float getEndTime() const { return end.time; }

  //! Returns normalized time if t is in range [start.time, end.time].
  inline float normalizeTime( float t ) const { return (t - start.time) / (end.time - start.time); }
  //! Returns the duration of this phrase.
  inline float getDuration() const { return end.time - start.time; }

  //! Returns the interpolated value at the given time.
  virtual T getValue( float atTime ) const
  {
    return lerpFn( start.value, end.value, motion( normalizeTime( atTime ) ) );
  }
private:
  Position<T> start;
  Position<T> end;
  EaseFn      motion;
  LerpFn      lerpFn = &lerpT<T>;
};

template<typename T>
using PhraseRef = std::shared_ptr<Phrase<T>>;

/**
 Phrase with separately-interpolated components.
 Allows for the use of separate ease functions per component.
 All components must be of the same type.
 */
template<typename T>
class Phrase2 : public Phrase<T>
{
public:
  Phrase2( const T &end, float duration, const EaseFn &ease_x, const EaseFn &ease_y ):
    Phrase<T>( end, duration ),
    motion_x( ease_x ),
    motion_y( ease_y )
  {}

  Phrase2( const Position<T> &start, const Position<T> &end, const EaseFn &ease_x, const EaseFn &ease_y ):
    Phrase<T>( start, end ),
    motion_x( ease_x ),
    motion_y( ease_y )
  {}

  //! Returns the interpolated value at the given time.
  T getValue( float atTime ) const override
  {
    float t = Phrase<T>::normalizeTime( atTime );
    return T( componentLerpFn( Phrase<T>::getStartValue().x, Phrase<T>::getEndValue().x, motion_x( t ) ),
             componentLerpFn( Phrase<T>::getStartValue().y, Phrase<T>::getEndValue().y, motion_y( t ) ) );
  }

  //! Set ease functions for first and second components.
  void setEase( const EaseFn &component_0, const EaseFn &component_1 ) {
    motion_x = component_0;
    motion_y = component_1;
  }

private:
  using ComponentT = decltype( T().x ); // get the type of the x component;
  using ComponentLerpFn = std::function<ComponentT (const ComponentT&, const ComponentT&, float)>;

  ComponentLerpFn componentLerpFn = &lerpT<ComponentT>;
  EaseFn          motion_x;
  EaseFn          motion_y;
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
  // Sequences always need to have some valid value.
  Sequence() = delete;

  //! Construct a Sequence with an initial \a value.
  explicit Sequence( const T &value ):
    _initial_value( value )
  {}

  //! Returns the Sequence value at \a atTime.
  T getValue( float atTime ) const;

  float getTimeWrapped( float time, float inflectionPoint = 0.0f ) const
  {
    if( time > getDuration() ) {
      return inflectionPoint + std::fmod( time, getDuration() - inflectionPoint );
    }
    else {
      return time;
    }
  }

  //! Returns the Sequence value at \a time, looping past the end from inflection point to the end.
  T getValueWrapped( float time, float inflectionPoint = 0.0f ) const
  {
    return getValue( getTimeWrapped( time, inflectionPoint ) );
  }

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
    Position<T> start{ value, _duration };
    Position<T> end{ value, _duration + duration };
    auto phrase = std::make_shared<Phrase<T>>( start, end, &easeHold );

    _segments.push_back( phrase );
    _duration = phrase->getEndTime();

    return *this;
  }

  //! Animate to \a value over \a duration seconds using \a ease easing.
  Sequence<T>& rampTo( const T &value, float duration, const EaseFn &ease = &easeNone )
  {
    Position<T> start{ endValue(), _duration };
    Position<T> end{ value, _duration + duration };
    auto phrase = std::make_shared<Phrase<T>>( start, end, ease );

    _segments.push_back( phrase );

    _duration = phrase->getEndTime();

    return *this;
  }

  template<typename PhraseT, typename... Args>
  Sequence<T>& then( const T& value, float duration, Args... args )
  {
    Position<T> start{ endValue(), _duration };
    Position<T> end{ value, _duration + duration };
    auto phrase = std::make_shared<PhraseT>( start, end, args... );
    // Would expect the following to make my dream user syntax work.
//    auto phrase = std::make_shared<PhraseT<T>>( start, end, args... );

    _segments.push_back( phrase );
    _duration = phrase->getEndTime();

    return *this;
  }

  Sequence<T>& then( const PhraseRef<T>& phrase )
  {
    phrase->setStartValue( endValue() );
    phrase->shiftStartTimeTo( _duration );
    _duration = phrase->getEndTime();

    _segments.push_back( phrase );

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
  T endValue() const { return _segments.empty() ? _initial_value : _segments.back()->getEndValue(); }

  //! Returns the value at the beginning of the Sequence.
  T initialValue() const { return _initial_value; }

private:
  std::vector<PhraseRef<T>>  _segments;
  T                       _initial_value;
  float                   _duration = 0.0f;

  friend class Timeline;
};

//! Returns the value of this sequence for a given point in time.
// Would be nice to have a constant-time check (without a while loop).
template<typename T>
T Sequence<T>::getValue( float atTime ) const
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
    if( (*iter)->getEndTime() > atTime )
    {
      return (*iter)->getValue( atTime );
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
