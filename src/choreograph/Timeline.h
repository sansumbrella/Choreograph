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
  /// Remove all elements from \a container that match \a compare
  template<class CONTAINER_TYPE, class COMPARATOR>
  void erase_if( CONTAINER_TYPE *container, COMPARATOR compare )
  {
    container->erase( std::remove_if( container->begin(),
                                     container->end(),
                                     compare ),
                     container->end() );
  }

  /// Remove all copies of \a element from \a vec
  template<class ELEMENT_TYPE>
  void vector_remove( std::vector<ELEMENT_TYPE> *vec, const ELEMENT_TYPE &element )
  {
    vec->erase( std::remove_if( vec->begin()
                               , vec->end()
                               , [element](const ELEMENT_TYPE &e){ return e == element; } )
               , vec->end() );
  }

//
// Motion options returned when you make a motion using the timeline.
// TODO: hide the underlying motion and sequence and provide a fresh interface on top.
// The MotionOptions interface will come around once other parts have solidified a bit.
//
template<typename T>
class MotionOptions
{
public:
  using SelfT = MotionOptions<T>;

  MotionOptions( const MotionRef<T> &motion, const SequenceRef<T> &sequence ):
    _motion( motion ),
    _sequence( sequence )
  {}

  //
  //  Motion Interface Mirroring.
  //

  /// Set function to be called when Motion starts. Receives reference to motion.
  SelfT& startFn( const typename Motion<T>::Callback &fn ) { _motion->startFn( fn ); return *this; }
  /// Set function to be called when Motion updates. Receives current target value.
  SelfT& updateFn( const typename Motion<T>::DataCallback &fn ) { _motion->updateFn( fn ); return *this; }
  /// Set function to be called when Motion finishes. Receives reference to motion.
  SelfT& finishFn( const typename Motion<T>::Callback &fn ) { _motion->finishFn( fn ); return *this; }
  /// Set the motion to be continuous, preventing it from being auto-removed from the timeline.
  SelfT& continuous( bool isContinuous ) { _motion->continuous( isContinuous ); return *this; }

  //
  //  Sequence Interface Mirroring.
  //

  /// Set the initial value of the Sequence or create and instantaneous hold of a value. 
  SelfT& set( const T &value ) { _sequence->set( value ); return *this; }
  /// Append a phrase to the Sequence.
  template<template <typename> class PhraseT, typename... Args>
  SelfT& then( const T &value, float duration, Args&&... args ) { _sequence->template then<PhraseT>( value, duration, std::forward<Args>(args)... ); return *this; }

private:
  MotionRef<T>   _motion;
  SequenceRef<T> _sequence;
};

/**
 Holds a collection of Motions and updates them through time.
 */
class Timeline
{
public:
  //=================================================
  // Creating Motions.
  //=================================================

  /// Apply a source to output, overwriting any previous connections.
  template<typename T>
  MotionOptions<T> apply( Output<T> *output )
  {
    auto sequence = std::make_shared<Sequence<T>>( *output );
    auto motion = std::make_shared<Motion<T>>( output, sequence );

    _motions.push_back( motion );

    return MotionOptions<T>{ motion, sequence };
  }

  /// Add phrases to the end of the Sequence currently connected to \a output.
  template<typename T>
  MotionOptions<T> append( Output<T> *output )
  {
    for( auto &m : _motions ) {
      if( m->getTarget() == output ) {
        auto motion = std::static_pointer_cast<Motion<T>>( m );
        return MotionOptions<T>{ motion, motion->template getSource<Sequence<T>>() };
      }
    }
    return apply( output );
  }

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

    return MotionOptions<T>{ motion, sequence };
  }

  /// Add phrases to the end of the Sequence currently connected to \a output. Raw pointer edition.
  /// Unless you have a strong need, prefer the use of append( Output<T> *output ) over this version.
  template<typename T>
  MotionOptions<T> append( T *output )
  {
    for( auto &m : _motions ) {
      if( m->getTarget() == output ) {
        auto motion = std::static_pointer_cast<Motion<T>>( m );
        return MotionOptions<T>{ motion, motion->template getSource<Sequence<T>>() };
      }
    }
    return apply( output );
  }

  //=================================================
  // Time manipulation.
  //=================================================

  /// Advance all current motions.
  void step( float dt );

  //=================================================
  // Timeline element manipulation.
  //=================================================

  /// Remove specific motion.
  void remove( const MotionBaseRef &motion );

  /// Remove motion associated with specific output.
  template<typename T>
  void remove( T *output )
  {
    erase_if( &_motions, [=] (const MotionBaseRef &m) { return m->getTarget() == output; } );
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

} // namespace choreograph
