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

using TimelineItemRef = std::shared_ptr<class TimelineItem>;

using TimelineItemUniqueRef = std::unique_ptr<class TimelineItem>;

using CueRef = std::shared_ptr<class Cue>;

template<typename T>
class Motion;

template<typename T>
using MotionRef = std::shared_ptr<Motion<T>>;


///
/// TimelineItem: non-templated base for polymorphic Motions.
/// Base class for anything that can go on a Timeline.
///
class TimelineItem
{
public:
  TimelineItem() = default;

  virtual ~TimelineItem() = default;

  //=================================================
  // Common public interface.
  //=================================================

  /// Advance motion in time. Affected by Motion's speed. Do not use from callbacks (it will fire them).
  void step( Time dt );

  /// Jump to a point in time. Ignores Motion's speed. Do not use from callbacks (it will fire them).
  void jumpTo( Time time );

  /// Set time. Ignores speed. Safe to use from callbacks.
  void setTime( Time time ) { _time = _previous_time = time; }

  //=================================================
  // Virtual Interface.
  //=================================================

  /// Overridden to determine what a time step does.
  virtual void update() = 0;

  /// Returns the duration of the motion.
  virtual Time getDuration() const = 0;

  /// Returns true if motion is valid.
  virtual bool isValid() const { return true; }

  /// Returns target if motion has one.
  virtual const void* getTarget() const { return nullptr; }

  //=================================================
  // Time manipulation and querying.
  //=================================================

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
  void resetTime();

  /// Returns the current end time of this motion.
  Time getEndTime() const { return getStartTime() + getDuration(); }

  /// Set the start time of this motion. Use to delay entire motion.
  void setStartTime( Time t ) { _start_time = t; }
  Time getStartTime() const { return _start_time; }

  /// Set whether the Motion should be removed from parent Timeline on finish.
  void setRemoveOnFinish( bool doRemove ) { _remove_on_finish = doRemove; }

  /// Returns true if the Motion should be removed from parent Timeline on finish.
  bool getRemoveOnFinish() const { return _remove_on_finish; }

private:
  /// True if this motion should be removed from Timeline on finish.
  bool       _remove_on_finish = true;
  /// Playback speed. Set to negative to go in reverse.
  Time       _speed = 1;
  /// Current animation time in seconds. Time at which Sequence is evaluated.
  Time       _time = 0;
  /// Previous animation time in seconds.
  Time       _previous_time = 0;
  /// Animation start time in seconds. Time from which Sequence is evaluated.
  /// Use to apply a delay.
  Time       _start_time = 0;
};

template<typename T>
struct MotionGroupOptions
{
  explicit MotionGroupOptions( Motion<T> &motion ):
    _motion( motion )
  {}

  /// Set a function to be called when we start the motion. Receives Motion as an argument.
  MotionGroupOptions& startFn( const typename Motion<T>::Callback &fn ) { _motion.setStartFn( fn ); return *this; }

  /// Set a function to be called on Motion update. Receives target as an argument.
  MotionGroupOptions& updateFn( const typename Motion<T>::DataCallback &fn ) { _motion.setUpdateFn( fn ); return *this; }

  /// Set a function to be called when we reach the end of the motion. Receives Motion as an argument.
  MotionGroupOptions& finishFn( const typename Motion<T>::Callback &fn ) { _motion.setFinishFn( fn ); return *this; }

private:
  Motion<T> &_motion;
};
///
/// Groups together a number of Motions so they can be repeated
/// and moved around together.
/// Note that grouped Motions all share the group's speed and time.
/// Assumes that the underlying Sequences won't change after adding.
///
class MotionGroup : public TimelineItem
{
public:
  using Callback = std::function<void (MotionGroup&)>;

  static std::unique_ptr<MotionGroup> create() { return std::unique_ptr<MotionGroup>( new MotionGroup ); }

  /// Create and add a Motion to the group.
  /// The Motion will apply \a sequence to \a output.
  template<typename T>
  MotionGroupOptions<T> add( const SequenceRef<T> &sequence, Output<T> *output );

  /// Update all grouped motions.
  void update() override;

  /// Returns true iff all grouped motions are valid.
  bool isValid() const override;

  /// Returns the duration of the motion group (end time of last motion).
  Time getDuration() const override { return _duration; }

  void setFinishFn( const Callback &fn ) { _finish_fn = fn; }
  void setStartFn( const Callback &fn ) { _start_fn = fn; }

private:
  MotionGroup() = default;
  Time                                _duration = 0;
  std::vector<TimelineItemUniqueRef>  _motions;

  Callback                            _start_fn = nullptr;
  Callback                            _finish_fn = nullptr;
};

///
/// Calls a function after time has elapsed.
///
class Cue : public TimelineItem
{
public:
  Cue() = delete;

  /// Control struct for cancelling Cues if needed.
  /// Accessible through the CueOptions struct.
  struct Control
  {
    /// Cancel the cue this belongs to.
    void cancel() { _valid = false; }
    bool _valid = true;
  };

  /// Creates a cue from a function and a delay.
  Cue( const std::function<void ()> &fn, Time delay );

  /// Returns a weak_ptr to a control that allows you to cancel the Cue.
  std::weak_ptr<Control> getControl() const { return _control; }

  /// Returns true if the cue should still execute.
  bool isValid() const override;

  /// Calls cue function if time threshold has been crossed.
  void update() override;

  /// Cues are instantaneous.
  Time getDuration() const override { return 0.0f; }
private:
  std::function<void ()>    _cue;
  std::shared_ptr<Control>  _control;
};

///
/// Motion: Moves a playhead along a Sequence and sends its value to a user-defined output.
/// Connects a Sequence and an Output.
///
template<typename T>
class Motion : public TimelineItem
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
  void setFinishFn( const Callback &c ) { _finishFn = c; }

  /// Set a function to be called when we start the sequence. Receives *this as an argument.
  void setStartFn( const Callback &c ) { _startFn = c; }

  /// Set a function to be called at each update step of the sequence.
  /// Function will be called immediately after setting the target value.
  void setUpdateFn( const DataCallback &c ) { _updateFn = c; }

  /// Update the connected target with the current sequence value.
  /// Calls start/update/finish functions as appropriate if assigned.
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
// MotionGroup Template Implementation.
//=================================================

template<typename T>
MotionGroupOptions<T> MotionGroup::add( const SequenceRef<T> &sequence, Output<T> *output )
{
  auto motion = std::unique_ptr<Motion<T>>( new Motion<T>( output, sequence ) );
  auto motion_ptr = motion.get();


  _motions.emplace_back( std::move( motion ) );

  _duration = std::max( _duration, motion_ptr->getDuration() );

  return MotionGroupOptions<T>( *motion_ptr );
}

//=================================================
// Motion Template Implementation.
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
    else if( backward() && time() < getDuration() && previousTime() >= getDuration() ) {
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
    if( forward() && time() >= getDuration() && previousTime() < getDuration() ) {
      _finishFn( *this );
    }
    else if( backward() && time() <= 0.0f && previousTime() > 0.0f ) {
      _finishFn( *this );
    }
  }
}

} // namespace choreograph
