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

#include "Source.h"
#include "Phrase.hpp"

namespace choreograph
{

/**
 A Sequence of motions.
 Our essential compositional tool, describing all the transformations to one element.
 A kind of platonic idea of an animation sequence; this describes a motion without giving it an output.
*/
template<typename T>
class Sequence : public Source<T>
{
public:
  // Sequences always need to have some valid value.
  Sequence() = delete;
  using SequenceT = Sequence<T>;

  /// Construct a Sequence with an initial \a value.
  explicit Sequence( const T &value ):
    Source<T>( 0, 0 ),
    _initial_value( value )
  {}

  /// Construct a Sequence from an array of Phrases.
  explicit Sequence( const std::vector<SourceRef<T>> &phrases ):
    Source<T>( phrases.front()->getStartTime(), phrases.back()->getEndTime() ),
    _initial_value( phrases.front()->getStartValue() ),
    _phrases( phrases )
  {}

  /// Returns the Sequence value at \a atTime.
  T getValue( float atTime ) const override;

  /// Set current value. An instantaneous hold.
  SequenceT& set( const T &value )
  {
    if( _phrases.empty() ) {
      _initial_value = value;
    }
    else {
      then<Hold>( value, 0.0f );
    }
    return *this;
  }

  /// Add a phrase to the end of the sequence.
  /// All phrases will receive the following arguments to their constructors:
  /// start time, end time, start value (last sequence value), end value
  /// If additional arguments are passed to then(), those arguments come after the required ones.
  /// sequence.then<RampTo>( targetValue, duration, other phrase parameters ).then<Hold>( holdValue, duration );
  template<template <typename> class PhraseT, typename... Args>
  SequenceT& then( const T &value, float duration, Args... args )
  {
    float end_time = this->getEndTime() + duration;
    _phrases.emplace_back( std::unique_ptr<PhraseT<T>>( new PhraseT<T>( this->getEndTime(), end_time, this->getEndValue(), value, std::forward<Args>(args)... ) ) );
    this->setEndTime( end_time );

    return *this;
  }

  /// Add a pre-existing phrase to the end of the sequence.
  template<typename PhraseT>
  SequenceT& then( const PhraseT &phrase )
  {
    auto p = std::unique_ptr<PhraseT>( new PhraseT( phrase ) );
    p->setStartTime( this->getEndTime() );
    this->setEndTime( p->getEndTime() );
    _phrases.push_back( std::move( p ) );

    return *this;
  }

  SequenceT& append( const SequenceT &next )
  {
    for( auto &phrase : next._phrases ) {
      then( phrase.get() );
    }
  }

  /// Returns the value at the end of the Sequence.
  T getEndValue() const override { return _phrases.empty() ? _initial_value : _phrases.back()->getEndValue(); }

  /// Returns the value at the beginning of the Sequence.
  T getStartValue() const override { return _initial_value; }

  /// Returns the number of phrases in the Sequence.
  size_t getPhraseCount() const { return _phrases.size(); }

  /// Add two sequences together to form a third sequence.
  static SequenceT concatenate( SequenceT lhs, const SequenceT &rhs )
  {
    return lhs.append( rhs );
  }

  /// Recursively concatenate any number of sequences.
  template<typename... Args>
  static SequenceT concatenate( const SequenceT &first, const SequenceT &second, Args... additional )
  {
    return concatenate( concatenate( first, second ), std::forward( additional... ) );
  }
protected:
  void startTimeShifted( float delta ) override
  {
    for( const SourceUniqueRef<T> &phrase : _phrases )
    {
      phrase->shiftTime( delta );
    }
  }

private:
  // We store unique pointers to phrases to prevent insanity when copying one sequence into another.
  // We would stack allocate these phrases, but we need pointers to enable polymorphic types.
  std::vector<SourceUniqueRef<T>> _phrases;
  T                         _initial_value;
};

/// Returns the value of this sequence for a given point in time.
// Would be nice to have a constant-time check (without a while loop).
template<typename T>
T Sequence<T>::getValue( float atTime ) const
{
  if( atTime < this->getStartTime() )
  {
    return _initial_value;
  }
  else if ( atTime >= this->getEndTime() )
  {
    return getEndValue();
  }

  auto iter = _phrases.begin();
  while( iter < _phrases.end() ) {
    if( (*iter)->getEndTime() > atTime )
    {
      return (*iter)->getValue( atTime );
    }
    ++iter;
  }
  // past the end, get the final value
  // this should be unreachable, given that we return early if time >= duration
  return getEndValue();
}

template<typename T>
using SequenceRef = std::shared_ptr<Sequence<T>>;

} // namespace choreograph
