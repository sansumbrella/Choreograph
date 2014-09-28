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
#include "detail/VectorManipulation.hpp"

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
  /// Set function to be called when Motion updates. Receives current target value.
  SelfT& updateFn( const typename Motion<T>::DataCallback &fn ) { _motion->updateFn( fn ); return *this; }
  /// Set function to be called when Motion finishes. Receives reference to motion.
  SelfT& finishFn( const typename Motion<T>::Callback &fn ) { _motion->finishFn( fn ); return *this; }
  /// Set the motion to be continuous, preventing it from being auto-removed from the timeline.
  SelfT& continuous( bool isContinuous ) { _motion->continuous( isContinuous ); return *this; }

  //=================================================
  // Sequence Interface Mirroring.
  //=================================================

  /// Set the current value of the Sequence. Acts as an instantaneous hold.
  SelfT& set( const T &value ) { _sequence->set( value ); return *this; }

  /// Append a phrase to the Sequence.
  template<template <typename> class PhraseT, typename... Args>
  SelfT& then( const T &value, float duration, Args&&... args ) { _sequence->template then<PhraseT>( value, duration, std::forward<Args>(args)... ); return *this; }

  /// Append a Hold to the end of the Sequence. Assumes you want to hold using the Sequence's current end value.
  SelfT& hold( float duration ) { _sequence->template then<Hold>( _sequence->getEndValue(), duration ); return *this; }

  //=================================================
  // Extra Sugar.
  //=================================================

  /// Set the start time of this motion to the current end of all motions of \a other.
  SelfT& after( void *other );

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
  MotionOptions<T> apply( Output<T> *output )
  {
    auto sequence = std::make_shared<Sequence<T>>( *output );
    auto motion = std::make_shared<Motion<T>>( output, sequence );

    _motions.push_back( motion );

    return MotionOptions<T>( motion, sequence, *this );
  }

  /// Apply a source to output, overwriting any previous connections.
  template<typename T>
  MotionOptions<T> apply( Output<T> *output, const SequenceRef<T> &sequence )
  {
    auto motion = std::make_shared<Motion<T>>( output, sequence );

    _motions.push_back( motion );

    return MotionOptions<T>( motion, sequence, *this );
  }

  /// Add phrases to the end of the Sequence currently connected to \a output.
  template<typename T>
  MotionOptions<T> append( Output<T> *output )
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

  //=================================================
  // Creating Motions. T* Versions.
  //=================================================

  /// Apply a source to output, overwriting any previous connections. Raw pointer edition.
  /// Unless you have a strong need, prefer the use of apply( Output<T> *output ) over this version.
  template<typename T>
  MotionOptions<T> apply( T *output )
  { // Remove any existing motions that affect the same variable.
    // This is a raw pointer, so we don't know about any prior relationships.
    remove( output );

    auto sequence = std::make_shared<Sequence<T>>( *output );
    auto motion = std::make_shared<Motion<T>>( output, sequence );

    _motions.push_back( motion );

    return MotionOptions<T>( motion, sequence, *this );
  }

  /// Apply a source to output, overwriting any previous connections.
  template<typename T>
  MotionOptions<T> apply( T *output, const SequenceRef<T> &sequence )
  { // Remove any existing motions that affect the same variable.
    remove( output );
    auto motion = std::make_shared<Motion<T>>( output, sequence );
    _motions.push_back( motion );

    return MotionOptions<T>( motion, sequence, *this );
  }

  /// Add phrases to the end of the Sequence currently connected to \a output. Raw pointer edition.
  /// Unless you have a strong need, prefer the use of append( Output<T> *output ) over this version.
  template<typename T>
  MotionOptions<T> append( T *output )
  {
    auto motion = find( output );
    if( motion ) {
      return MotionOptions<T>( motion, motion->getSequence(), *this );
    }
    return apply( output );
  }

  //=================================================
  // Creating Cues.
  //=================================================

  /// Add a cue to the timeline. It will be called after \a delay time elapses.
  void cue( const std::function<void ()> &fn, float delay );

  //=================================================
  // Time manipulation.
  //=================================================

  /// Advance all current motions.
  void step( float dt );

  /// Set all motions to \a time.
  void jumpTo( float time );

  //=================================================
  // Timeline element manipulation.
  //=================================================

  template<typename T>
  MotionRef<T> find( T *output )
  {
    for( auto &m : _motions ) {
      if( m->getTarget() == output ) {
        return std::static_pointer_cast<Motion<T>>( m );
      }
    }
    return nullptr;
  }

  /// Remove specific motion.
  void remove( const MotionBaseRef &motion );

  /// Remove motion associated with specific output.
  template<typename T>
  void remove( T *output )
  {
    detail::erase_if( &_motions, [=] (const MotionBaseRef &m) { return m->getTarget() == output; } );
  }

  /// Remove all motions from this timeline.
  void clear() { _motions.clear(); }

  /// Return true iff there are no motions on this timeline.
  bool empty() const { return _motions.empty(); }

  /// Return the number of motions on this timeline.
  size_t size() const { return _motions.size(); }

private:
  // True if Motions should be removed from timeline when they reach their endTime.
  bool                        _auto_clear = true;
  std::vector<MotionBaseRef>  _motions;
};

//=================================================
// Additional MotionOptions Implementation.
//=================================================

template<typename T>
MotionOptions<T>& MotionOptions<T>::after( void *other )
{
  auto ptr = _timeline.find( other );
  if( ptr ) {
    _motion->setStartTime( ptr->getEndTime() );
  }
  return *this;
}


} // namespace choreograph
