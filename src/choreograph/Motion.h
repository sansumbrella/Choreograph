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

#include "Sequence.hpp"
#include "Connection.hpp"
#include "Output.hpp"

namespace choreograph
{

//=================================================
// Aliases.
//=================================================

using MotionBaseRef = std::shared_ptr<class MotionBase>;

template<typename T>
class Motion;

template<typename T>
using MotionRef = std::shared_ptr<Motion<T>>;


///
/// MotionBase: non-templated base for polymorphic Motions.
/// Connects a Sequence and an Output.
///
class MotionBase
{
public:
  MotionBase() = default;

  /// Advance motion in time. Affected by Motion's speed.
  void step( Time dt );

  /// Jump to a point in time. Ignores Motion's speed.
  void jumpTo( Time time );

  /// Overridden to determine what a time step does.
  virtual void update() = 0;

  /// Returns the duration of the motion.
  virtual Time getDuration() const = 0;

  /// Returns true if motion is valid.
  virtual bool isValid() const { return true; }

  /// Returns target if motion has one.
  virtual const void* getTarget() const { return nullptr; }

  /// Returns current animation time in seconds.
  Time time() const { return _time - _start_time; }

  /// Returns previous step's animation time in seconds.
  Time previousTime() const { return _previous_time - _start_time; }

  /// Returns true if animation plays forward with positive time steps.
  bool  forward() const { return _speed >= 0.0f; }

  /// Returns true if animation plays backward with positive time steps.
  bool  backward() const { return _speed < 0.0f; }

  /// Returns true if this Motion's time is past the end of its duration. Accounts for reversed playback.
  bool  isFinished() const;

  /// Set playback speed of motion. Use negative numbers to play in reverse.
  void  setPlaybackSpeed( Time s ) { _speed = s; }

  /// Returns the current playback speed of motion.
  Time getPlaybackSpeed() const { return _speed; }

  /// Reset motion to beginning. Accounts for reversed playback.
  void  resetTime();

  /// Returns the current end time of this motion.
  Time getEndTime() const { return getStartTime() + getDuration(); }

  /// Set the start time of this motion. Use to delay entire motion.
  void setStartTime( Time t ) { _start_time = t; }
  Time getStartTime() const { return _start_time; }

protected:
  // True if the underlying Sequence should play forever.
  bool        _continuous = false;
private:
  /// Playback speed. Set to negative to go in reverse.
  Time       _speed = 1.0f;
  /// Current animation time in seconds. Time at which Sequence is evaluated.
  Time       _time = 0.0f;
  /// Previous animation time in seconds.
  Time       _previous_time = 0.0f;
  /// Animation start time in seconds. Time from which Sequence is evaluated.
  /// Use to apply a delay.
  Time       _start_time = 0.0f;
};

class Cue : public MotionBase
{
public:
  Cue() = delete;

  /// Creates a cue from a function and a delay.
  Cue( const std::function<void ()> &fn, Time delay );

  /// Calls cue function if time threshold has been crossed.
  void update() override;

  /// Cues are instantaneous.
  Time getDuration() const override { return 0.0f; }
private:
  std::function<void ()> _cue;
};

///
/// Motion: Moves a playhead along a Sequence and sends its value to a user-defined output.
///
template<typename T>
class Motion : public MotionBase
{
public:
  using MotionT       = Motion<T>;
  using SequenceRefT  = SequenceRef<T>;
  using DataCallback  = std::function<void (T&)>;
  using Callback      = std::function<void (MotionT&)>;

  Motion() = delete;

  Motion( T *target, const SequenceRefT &sequence ):
    _connection( target ),
    _source( sequence )
  {}

  Motion( Output<T> *target, const SequenceRefT &sequence ):
    _connection( target ),
    _source( sequence )
  {}

  /// Returns duration of the underlying sequence.
  Time getDuration() const override { return _source->getDuration(); }

  /// Returns ratio of time elapsed, from [0,1] over duration.
  Time getProgress() const { return time() / _source->getDuration(); }

  /// Returns the underlying Sequence sampled for this motion.
  SequenceRefT  getSequence() { return _source; }

  bool isValid() const override { return _connection.isConnected(); }
  const void* getTarget() const override { return _connection.targetPtr(); }

  /// Set a function to be called when we reach the end of the sequence. Receives *this as an argument.
  MotionT&  finishFn( const Callback &c ) { _finishFn = c; return *this; }

  /// Set a function to be called when we start the sequence. Receives *this as an argument.
  MotionT&  startFn( const Callback &c ) { _startFn = c; return *this; }

  /// Set a function to be called at each update step of the sequence. Called immediately after setting the target value.
  MotionT&  updateFn( const DataCallback &c ) { _updateFn = c; return *this; }

  /// Set the playback speed of this motion. Use negative numbers for reverse.
  MotionT&  playbackSpeed( Time s ) { setPlaybackSpeed( s ); return *this; }

  /// Set the connection to play continuously.
  MotionT&  continuous( bool c ) { _continuous = c; return *this; }

  /// Update
  void update() override;

private:
  // shared_ptr to source since many connections could share the same source.
  // This enables us to do pseudo-instancing on our animations, reducing their memory footprint.
  SequenceRefT    _source;
  Connection<T>   _connection;

  Callback        _finishFn = nullptr;
  Callback        _startFn  = nullptr;
  DataCallback    _updateFn = nullptr;
};

//=================================================
// Motion Implementation.
//=================================================

template<typename T>
void Motion<T>::update()
{
  assert( this->isValid() );

  if( _startFn )
  {
    if( forward() && time() > 0.0f && previousTime() <= 0.0f ) {
      _startFn( *this );
    }
    else if( backward() && time() < _source->getDuration() && previousTime() >= _source->getDuration() ) {
      _startFn( *this );
    }
  }

  _connection.target() = _source->getValue( time() );

  if( _updateFn )
  {
    _updateFn( _connection.target() );
  }

  if( _finishFn )
  {
    if( forward() && time() >= _source->getDuration() && previousTime() < _source->getDuration() ) {
      _finishFn( *this );
    }
    else if( backward() && time() <= 0.0f && previousTime() > 0.0f ) {
      _finishFn( *this );
    }
  }
}

} // namespace choreograph
