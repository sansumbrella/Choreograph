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

  MotionOptions( const MotionRef<T> &motion, const SequenceRef<T> &sequence, const Timeline &timeline ):
    _motion( motion ),
    _sequence( sequence ),
    _timeline( timeline )
  {}

  //=================================================
  // Motion Interface Mirroring.
  //=================================================

  /// Set function to be called when Motion starts. Receives reference to motion.
  SelfT& startFn( const typename Motion<T>::Callback &fn ) { _motion->startFn( fn ); return *this; }
  /// Set function to be called when Motion starts. Receives no arguments.
  SelfT& startFn( const typename Motion<T>::EmptyCallback &fn ) { _motion->startFn( fn ); return *this; }

  /// Set function to be called when Motion updates. Receives current target value.
  SelfT& updateFn( const typename Motion<T>::DataCallback &fn ) { _motion->updateFn( fn ); return *this; }

  /// Set function to be called when Motion finishes. Receives reference to motion.
  SelfT& finishFn( const typename Motion<T>::Callback &fn ) { _motion->finishFn( fn ); return *this; }
  /// Set function to be called when Motion finishes. Receives no arguments.
  SelfT& finishFn( const typename Motion<T>::EmptyCallback &fn ) { _motion->finishFn( fn ); return *this; }

  /// Set the motion to be continuous, preventing it from being auto-removed from the timeline.
  SelfT& continuous( bool isContinuous ) { _motion->continuous( isContinuous ); return *this; }

  SelfT& setStartTime( Time t ) { _motion->setStartTime( t ); return *this; }

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
  template<typename PhraseT>
  SelfT& then( const PhraseT &phrase ) { _sequence->template then<PhraseT>( phrase ); return *this; }

  //=================================================
  // Extra Sugar.
  //=================================================

  /// Append a Hold to the end of the Sequence. Assumes you want to hold using the Sequence's current end value.
  SelfT& hold( Time duration ) { _sequence->template then<Hold>( _sequence->getEndValue(), duration ); return *this; }

  /// Set the start time of this motion to the current end of all motions of \a other.
  template<typename U>
  SelfT& after( U *other );

  SelfT& delay( Time t ) { _motion->setStartTime( _motion->getStartTime() + t ); return *this; }

private:
  MotionRef<T>    _motion;
  SequenceRef<T>  _sequence;
  const Timeline  &_timeline;
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

  /// Add phrases to the end of the Sequence currently connected to \a output.
  template<typename T>
  MotionOptions<T> append( Output<T> *output );

  /// Build a Motion Sequence for \a output to be started on a Cue.
  /// Will create and invalid Motion, then make it valid with a Cue.
  /// Either allow Outputs multiple Connections (with one active at a time), or this will be dangerous.
  /// Not yet implemented.
  template<typename T>
  MotionOptions<T> later( Output<T> *output );

  //=================================================
  // Creating Motions. T* Versions.
  //=================================================

  /// Apply a source to output, overwriting any previous connections. Raw pointer edition.
  /// Unless you have a strong need, prefer the use of apply( Output<T> *output ) over this version.
  template<typename T>
  MotionOptions<T> apply( T *output );

  /// Apply a source to output, overwriting any previous connections.
  template<typename T>
  MotionOptions<T> apply( T *output, const SequenceRef<T> &sequence );

  /// Add phrases to the end of the Sequence currently connected to \a output. Raw pointer edition.
  /// Unless you have a strong need, prefer the use of append( Output<T> *output ) over this version.
  template<typename T>
  MotionOptions<T> append( T *output );

  //=================================================
  // Creating Cues.
  //=================================================

  /// Add a cue to the timeline. It will be called after \a delay time elapses.
  void cue( const std::function<void ()> &fn, Time delay );

  //=================================================
  // Time manipulation.
  //=================================================

  /// Advance all current motions.
  void step( Time dt );

  /// Set all motions to \a time.
  void jumpTo( Time time );

  //=================================================
  // Timeline element manipulation.
  //=================================================

  template<typename T>
  MotionRef<T> find( T *output ) const;

  /// Remove specific motion.
  void remove( const MotionBaseRef &motion );

  /// Remove motion associated with specific output.
  void remove( void *output );

  /// Set whether motions should be removed when finished. Default is true.
  void setAutoRemove( bool doRemove = true ) { _auto_clear = doRemove; }

  /// Remove all motions from this timeline.
  void clear() { _motions.clear(); }

  /// Return true iff there are no motions on this timeline.
  bool empty() const { return _motions.empty(); }

  /// Return the number of motions on this timeline.
  size_t size() const { return _motions.size(); }

  Time getDuration() const;

private:
  // True if Motions should be removed from timeline when they reach their endTime.
  bool                        _auto_clear = true;
  std::vector<MotionBaseRef>  _motions;
};

//=================================================
// Timeline Template Function Implementation.
//=================================================

template<typename T>
MotionOptions<T> Timeline::apply( Output<T> *output )
{
  auto sequence = std::make_shared<Sequence<T>>( *output );
  auto motion = std::make_shared<Motion<T>>( output, sequence );

  _motions.push_back( motion );

  return MotionOptions<T>( motion, sequence, *this );
}

template<typename T>
MotionOptions<T> Timeline::apply( Output<T> *output, const SequenceRef<T> &sequence )
{
  auto motion = std::make_shared<Motion<T>>( output, sequence );

  _motions.push_back( motion );

  return MotionOptions<T>( motion, sequence, *this );
}

template<typename T>
MotionOptions<T> Timeline::append( Output<T> *output )
{
  if( output->isConnected() )
  {
    auto motion = find( output->valuePtr() );
    if( motion ) {
      return MotionOptions<T>( motion, motion->getSequence(), *this );
    }
  }
  return apply( output );
}

template<typename T>
MotionOptions<T> Timeline::apply( T *output )
{ // Remove any existing motions that affect the same variable.
  // This is a raw pointer, so we don't know about any prior relationships.
  remove( output );

  auto sequence = std::make_shared<Sequence<T>>( *output );
  auto motion = std::make_shared<Motion<T>>( output, sequence );

  _motions.push_back( motion );

  return MotionOptions<T>( motion, sequence, *this );
}

template<typename T>
MotionOptions<T> Timeline::apply( T *output, const SequenceRef<T> &sequence )
{ // Remove any existing motions that affect the same variable.
  remove( output );
  auto motion = std::make_shared<Motion<T>>( output, sequence );
  _motions.push_back( motion );

  return MotionOptions<T>( motion, sequence, *this );
}

template<typename T>
MotionOptions<T> Timeline::append( T *output )
{
  auto motion = find( output );
  if( motion ) {
    return MotionOptions<T>( motion, motion->getSequence(), *this );
  }
  return apply( output );
}

template<typename T>
MotionRef<T> Timeline::find( T *output ) const
{
  for( auto &m : _motions ) {
    if( m->getTarget() == output ) {
      return std::static_pointer_cast<Motion<T>>( m );
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
