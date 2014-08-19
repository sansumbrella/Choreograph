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

#include "pockets/CollectionUtilities.hpp"

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

// A position describes a point in time.

template<typename T>
struct Position
{
  T     value;
  float time;
};

template<typename T>
T lerpT( const T &a, const T &b, float t )
{
  return a + (b - a) * t;
}

/*
 A Phrase is a part of a Sequence.
 It describes the motion between two positions.
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

/*
 A Sequence of motions.
 Our essential compositional tool, describing all the transformations to one element.
 The platonic idea of an animation sequence; this describes a motion for all possible points in time.
*/
template<typename T>
class Sequence
{
public:
  T getValue( float atTime );

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

  Sequence<T>& hold( float duration )
  {
    return hold( endValue(), duration );
  }

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

  float getDuration() const { return _duration; }

private:
  std::vector<Phrase<T>>  _segments;
  T     _initial_value;
  T     endValue() const { return _segments.empty() ? _initial_value : _segments.back().end.value; }
  Phrase<T>& endPhrase() const { return _segments.back(); }
  float _duration = 0.0f;

  friend class Timeline;
};

//! Returns the value of this sequence for a given point in time.
// Is there a way to get the value in constant time (without a while loop)?
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

/**
 A connection between a continuous, independent Sequence and an output.
 Non-templated base type so we can store in a polymorphic container.
 */
class MotionBase
{
public:
  virtual ~MotionBase() = default;

  //! Move forward in time.
  void step( float dt )
  {
    _time += dt * _speed;
    update();
    _previous_time = _time;
  }
  virtual void update() = 0;

  virtual float getDuration() const = 0;

  //! Returns current animation time in seconds.
  float time() const { return _time - _start_time; }
  //! Returns previous step's animation time in seconds.
  float previousTime() const { return _previous_time - _start_time; }

  bool forward() const { return _speed > 0.0f; }
  bool backward() const { return _speed < 0.0f; }

  // True if the underlying Sequence should play forever.
  bool        _continuous = false;

  void        *_target = nullptr;

  void        setPlaybackSpeed( float s ) { _speed = s; }
  float       getPlaybackSpeed() const { return _speed; }

private:
  // Playback speed. Set to negative to go in reverse.
  float       _speed = 1.0f;
  // Current animation time in seconds.
  float       _time = 0.0f;
  // Previous animation time in seconds.
  float       _previous_time = 0.0f;
  // Time from which Sequence is evaluated.
  float       _start_time = 0.0f;
};

class Cue : public MotionBase
{
public:
  void update() override
  {
    if( forward() && time() >= 0.0f && previousTime() < 0.0f )
      _cue();
    else if( forward() && time() <= 0.0f && previousTime() > 0.0f )
      _cue();
  }
private:
  std::function<void ()> _cue;
};

/**
 Motion: a sequence embedded in time and connected to an output.
 A connection between a continuous, independent Sequence and an output.
 Drives a Sequence and sends its value to a user-defined variable.
 Might mirror the Sequence interface for easier animation.
 */
template<typename T>
class Motion : public MotionBase
{
public:
  void update() override
  {
    if( _startFn ) {
      if( forward() && time() > 0.0f && previousTime() <= 0.0f )
        _startFn( *this );
      else if( backward() && time() < sequence->getDuration() && previousTime() >= sequence->getDuration() )
        _startFn( *this );
    }

    *output = sequence->getValue( time() );
    if( _updateFn ) {
      _updateFn( *output );
    }

    if( _finishFn ){
      if( forward() && time() >= sequence->getDuration() && previousTime() < sequence->getDuration() )
        _finishFn( *this );
      else if( backward() && time() <= 0.0f && previousTime() > 0.0f )
        _finishFn( *this );
    }
  }

  void* getOutput() const { return output; }
  float getDuration() const override { return sequence->getDuration(); }

  float getProgress() const { return time() / sequence->getDuration(); }

  //! Returns the underlying sequence for extension.
  Sequence<T>&  getSequence() { return *sequence; }

  typedef std::function<void (const T&)>        DataCallback;
  typedef std::function<void (Motion<T> &)> Callback;
  //! Set a function to be called when we reach the end of the sequence.
  Motion<T>& finishFn( const Callback &c ) { _finishFn = c; return *this; }
  //! Set a function to be called when we start the sequence.
  Motion<T>& startFn( const Callback &c ) { _startFn = c; return *this; }
  //! Set a function to be called at each update step of the sequence. Called immediately after setting the target value.
  Motion<T>& updateFn( const DataCallback &c ) { _updateFn = c; return *this; }

  Motion<T>&    playbackSpeed( float s ) { setPlaybackSpeed( s ); return *this; }

  //! Set the connection to play continuously.
  Motion<T>& continuous( bool c ) { _continuous = c; return *this; }

  // shared_ptr to sequence since many connections could share the same sequence
  // this enables us to to pseudo-instancing on our animations, reducing their memory footprint.
  std::shared_ptr<Sequence<T>> sequence;
  T             *output;
  Callback      _finishFn = nullptr;
  Callback      _startFn = nullptr;
  DataCallback  _updateFn = nullptr;

};

/*
 Holds a collection of connected sequences.
 Maybe variadic templates to specify an animation with different channel types,
 or one composed of n existing channels...
 */
class Timeline
{
public:

  //! Create a Sequence that is connected out to \a output.
  template<typename T>
  Motion<T>& move( T *output )
  { // remove any existing motions that affect the same variable (because that doesn't make sense within a single timeline)
    pk::vector_erase_if( &_motions, [=] (std::shared_ptr<MotionBase> m) { return m->_target == output; } );

    auto c = std::make_shared<Motion<T>>();
    c->sequence = std::make_shared<Sequence<T>>();
    c->output = output;
    c->_target = output;
    c->sequence->_initial_value = *output;
    _motions.push_back( c );
    return *c;
  }

  //! Create a Motion that plays \a sequence into \a output.
  template<typename T>
  Motion<T>& move( T *output, std::shared_ptr<Sequence<T>> sequence )
  { // remove any existing motions that affect the same variable (because that doesn't make sense within a single timeline)
    pk::vector_erase_if( &_motions, [=] (const std::shared_ptr<MotionBase> &m) { return m->_target == output; } );

    auto c = std::make_shared<Motion<T>>();
    c->sequence = sequence;
    c->output = output;
    c->_target = output;
    c->sequence->_initial_value = *output;
    _motions.push_back( c );
    return *c;
  }

  // Advance all current connections.
  void step( float dt )
  {
    for( auto &c : _motions )
    {
      c->step( dt );
    }

    if( _auto_clear )
    {
      pk::vector_erase_if( &_motions, [=] (const std::shared_ptr<MotionBase> &c ) { return !c->_continuous && c->time() >= c->getDuration(); } );
//      _motions.erase( std::remove_if( _motions.begin(), _motions.end(), [=] (const std::shared_ptr<MotionBase> &c ) { return !c->_continuous && c->time >= c->getDuration(); } ), _motions.end() );
    }
  }

  void remove( const std::shared_ptr<MotionBase> &motion )
  {
    _motions.erase( std::remove_if( _motions.begin(), _motions.end(), [=] (const std::shared_ptr<MotionBase> &c ) { return c == motion; } ), _motions.end() );
  }

private:
  bool                                      _auto_clear = true;
  std::vector<std::shared_ptr<MotionBase>>  _motions;
};

} // namespace choreograph

namespace co = choreograph;
