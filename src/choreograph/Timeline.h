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
#include "Motion.h"

namespace choreograph
{

class Timeline;

///
/// MotionOptions provide a facade for manipulating a timeline Motion and its underlying Sequence.
/// All methods return a reference back to the MotionOptions object for chaining.
///
template<typename T>
class MotionOptions
{
public:
  using SelfT = MotionOptions<T>;

  MotionOptions( Motion<T> *motion, Sequence<T> *sequence, const Timeline &timeline ):
    _motion( motion ),
    _sequence( sequence ),
    _timeline( timeline )
  {}

  //=================================================
  // Motion Interface Mirroring.
  //=================================================

  /// Set function to be called when Motion starts. Receives reference to motion.
  SelfT& startFn( const typename Motion<T>::Callback &fn ) { _motion->setStartFn( fn ); return *this; }

  /// Set function to be called when Motion updates. Receives current target value.
  SelfT& updateFn( const typename Motion<T>::DataCallback &fn ) { _motion->setUpdateFn( fn ); return *this; }

  /// Set function to be called when Motion finishes. Receives reference to motion.
  SelfT& finishFn( const typename Motion<T>::Callback &fn ) { _motion->setFinishFn( fn ); return *this; }

  /// Set whether the motion should be removed from the timeline on finish.
  SelfT& removeOnFinish( bool doRemove ) { _motion->setRemoveOnFinish( doRemove ); return *this; }

  /// Set the Motion's start time. Only useful after an apply() call.
  SelfT& setStartTime( Time t ) { _motion->setStartTime( t ); return *this; }

  /// Set the rate at which time advances for Motion.
  SelfT& playbackSpeed( Time speed ) { _motion->setPlaybackSpeed( speed ); return *this; }

  //=================================================
  // Sequence Interface Mirroring.
  //=================================================

  /// Set the current value of the Sequence. Acts as an instantaneous hold.
  SelfT& set( const T &value ) { _sequence->set( value ); return *this; }

  /// Append a new phrase to the Sequence.
  template<template <typename> class PhraseT, typename... Args>
  SelfT& then( const T &value, Time duration, Args&&... args ) { _sequence->template then<PhraseT>( value, duration, std::forward<Args>(args)... ); return *this; }

  /// Clone and append a phrase to the Sequence.
  SelfT& then( const PhraseRef<T> &phrase ) { _sequence->then( phrase ); return *this; }

  //=================================================
  // Extra Sugar.
  //=================================================

  /// Append a Hold to the end of the Sequence. Assumes you want to hold using the Sequence's current end value.
  SelfT& hold( Time duration ) { _sequence->template then<Hold>( _sequence->getEndValue(), duration ); return *this; }

  /// Set the start time of this motion to the current end of all motions of \a other.
  template<typename U>
  SelfT& after( U *other );

  /// Offset the Motion's start time.
  SelfT& shiftStartTime( Time t ) { _motion->setStartTime( _motion->getStartTime() + t ); return *this; }

private:
  Motion<T>       *_motion;
  Sequence<T>     *_sequence;
  const Timeline  &_timeline;
};

///
/// CueOptions provide a facade for manipulating a timeline Cue.
/// All methods return a reference back to the CueOptions object for chaining.
///
class CueOptions
{
public:
  explicit CueOptions( Cue &cue ):
    _cue( cue )
  {}
  /// Set the Cue to be removed from the timeline when finished.
  CueOptions& removeOnFinish( bool doRemove ) { _cue.setRemoveOnFinish( doRemove ); return *this; }

  /// Change the time (from now) at which the Cue will be called.
  CueOptions& setStartTime( Time t ) { _cue.setStartTime( t ); return *this; }

  /// Change the rate at which time flows toward the Cue's execution.
  CueOptions& playbackSpeed( Time speed ) { _cue.setPlaybackSpeed( speed ); return *this; }
private:
  Cue  &_cue;
};

/**
 Holds a collection of Motions and updates them through time.
 */
class Timeline
{
public:
  //=================================================
  // Creating Motions. Output<T>* Versions
  //=================================================

  /// Apply a source to output, overwriting any previous connections.
  template<typename T>
  MotionOptions<T> apply( Output<T> *output );

  /// Apply a source to output, overwriting any previous connections.
  template<typename T>
  MotionOptions<T> apply( Output<T> *output, const SequenceRef<T> &sequence );

  template<typename T>
  MotionOptions<T> apply( Output<T> *output, const PhraseRef<T> &phrase );

  /// Add phrases to the end of the Sequence currently connected to \a output.
  template<typename T>
  MotionOptions<T> append( Output<T> *output );

/*
  /// Build a Motion Sequence for \a output to be started on a Cue.
  /// Will create and invalid Motion, then make it valid with a Cue.
  /// Either allow Outputs multiple Connections (with one active at a time), or this will be dangerous.
  /// Not yet implemented. May never be implemented.
  /// Generally, we don't want anything to be on the timeline that the timeline doesn't know about. Believe.
  template<typename T>
  MotionOptions<T> later( Output<T> *output );
*/

  //=================================================
  // Creating Motions. T* Versions.
  //=================================================

  /// Apply a source to output, overwriting any previous connections. Raw pointer edition.
  /// Unless you have a strong need, prefer the use of apply( Output<T> *output ) over this version.
  template<typename T>
  MotionOptions<T> applyRaw( T *output );

  /// Apply a source to output, overwriting any previous connections.
  template<typename T>
  MotionOptions<T> applyRaw( T *output, const SequenceRef<T> &sequence );

  /// Add phrases to the end of the Sequence currently connected to \a output. Raw pointer edition.
  /// Unless you have a strong need, prefer the use of append( Output<T> *output ) over this version.
  template<typename T>
  MotionOptions<T> appendRaw( T *output );

  //=================================================
  // Creating Cues.
  //=================================================

  /// Add a cue to the timeline. It will be called after \a delay time elapses on this Timeline.
  CueOptions cue( const std::function<void ()> &fn, Time delay );

  //=================================================
  // Adding other things.
  //=================================================

  /// Add Motion to timeline.
  void add( TimelineItemUniqueRef motion );

  //=================================================
  // Time manipulation.
  //=================================================

  /// Advance all current motions.
  void step( Time dt );

  /// Set all motions to \a time.
  void jumpTo( Time time );

  /// Set all motions to \a time. Safe to call from Motion callbacks.
  void setTime( Time time );

  //=================================================
  // Timeline element manipulation.
  //=================================================

  /// Returns a non-owning raw pointer to the Motion applied to \a output, if any.
  /// If there is no Motion applied, returns nullptr.
  template<typename T>
  Motion<T>* find( T *output ) const;

  /// Remove motion associated with specific output.
  void remove( void *output );

  /// Set whether motions should be removed when finished. Default is true.
  /// This value will be passed to all future Motions created by the timeline.
  void setDefaultRemoveOnFinish( bool doRemove = true ) { _default_remove_on_finish = doRemove; }

  /// Remove all motions from this timeline.
  void clear() { _motions.clear(); }

  /// Return true iff there are no motions on this timeline.
  bool empty() const { return _motions.empty(); }

  /// Return the number of motions on this timeline.
  size_t size() const { return _motions.size(); }

private:
  // True if Motions should be removed from timeline when they reach their endTime.
  bool                                _default_remove_on_finish = true;
  std::vector<TimelineItemUniqueRef>  _motions;
};

//=================================================
// Timeline Template Function Implementation.
//=================================================

template<typename T>
MotionOptions<T> Timeline::apply( Output<T> *output )
{
  auto sequence = std::make_shared<Sequence<T>>( *output );
  auto motion = std::unique_ptr<Motion<T>>( new Motion<T>( output, sequence ) );
  motion->setRemoveOnFinish( _default_remove_on_finish );

  auto motion_ptr = motion.get();
  _motions.emplace_back( std::move( motion ) );

  return MotionOptions<T>( motion_ptr, sequence.get(), *this );
}

template<typename T>
MotionOptions<T> Timeline::apply( Output<T> *output, const PhraseRef<T> &phrase )
{
  auto sequence = std::make_shared<Sequence<T>>( phrase );
  auto motion = std::unique_ptr<Motion<T>>( new Motion<T>( output, sequence ) );
  motion->setRemoveOnFinish( _default_remove_on_finish );

  auto motion_ptr = motion.get();
  _motions.emplace_back( std::move( motion ) );

  return MotionOptions<T>( motion_ptr, sequence.get(), *this );
}

template<typename T>
MotionOptions<T> Timeline::apply( Output<T> *output, const SequenceRef<T> &sequence )
{
  auto motion = std::unique_ptr<Motion<T>>( new Motion<T>( output, sequence ) );
  motion->setRemoveOnFinish( _default_remove_on_finish );

  auto motion_ptr = motion.get();
  _motions.emplace_back( std::move( motion ) );

  return MotionOptions<T>( motion_ptr, sequence.get(), *this );
}

template<typename T>
MotionOptions<T> Timeline::append( Output<T> *output )
{
  if( output->isConnected() )
  {
    auto motion = find( output->valuePtr() );
    if( motion ) {
      return MotionOptions<T>( motion, motion->getSequence().get(), *this );
    }
  }
  return apply( output );
}

template<typename T>
MotionOptions<T> Timeline::applyRaw( T *output )
{ // Remove any existing motions that affect the same variable.
  // This is a raw pointer, so we don't know about any prior relationships.
  remove( output );

  auto sequence = std::make_shared<Sequence<T>>( *output );
  auto motion = std::unique_ptr<Motion<T>>( new Motion<T>( output, sequence ) );
  motion->setRemoveOnFinish( _default_remove_on_finish );

  auto motion_ptr = motion.get();
  _motions.emplace_back( std::move( motion ) );

  return MotionOptions<T>( motion_ptr, sequence.get(), *this );
}

template<typename T>
MotionOptions<T> Timeline::applyRaw( T *output, const SequenceRef<T> &sequence )
{ // Remove any existing motions that affect the same variable.
  remove( output );
  auto motion = std::unique_ptr<Motion<T>>( new Motion<T>( output, sequence ) );
  motion->setRemoveOnFinish( _default_remove_on_finish );

  auto motion_ptr = motion.get();
  _motions.emplace_back( std::move( motion ) );

  return MotionOptions<T>( motion_ptr, sequence.get(), *this );
}

template<typename T>
MotionOptions<T> Timeline::appendRaw( T *output )
{
  auto motion = find( output );
  if( motion ) {
    return MotionOptions<T>( motion.get(), motion->getSequence().get(), *this );
  }
  return apply( output );
}

template<typename T>
Motion<T>* Timeline::find( T *output ) const
{
  for( auto &m : _motions ) {
    if( m->getTarget() == output ) {
      return static_cast<Motion<T>*>( m.get() );
    }
  }
  return nullptr;
}

//=================================================
// Additional MotionOptions Implementation.
//=================================================

template<typename T>
template<typename U>
MotionOptions<T>& MotionOptions<T>::after( U *other )
{
  auto ptr = _timeline.find( other );
  if( ptr ) {
    _motion->setStartTime( ptr->getEndTime() );
  }
  return *this;
}


} // namespace choreograph
