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

#include "Source.hpp"
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

  /// Construct a Sequence with an initial \a value.
  explicit Sequence( const T &value ):
    Source<T>( 0, 0 ),
    _initial_value( value )
  {}

  /// Construct a Sequence with and initial \a value.
  explicit Sequence( T &&value ):
    Source<T>( 0, 0 ),
    _initial_value( std::forward<T>( value ) )
  {}

  /// Construct a Sequence by duplicating the phrases in an \a other sequence.
  Sequence( const Sequence<T> &other ):
    _initial_value( other._initial_value )
  {
    then( other );
  }

  //
  // Sequence manipulation and expansion.
  //

  /// Set the end \a value of Sequence.
  /// If there are no Phrases, this is the initial value.
  /// Otherwise, this is and instantaneous hold at \a value.
  Sequence<T>& set( const T &value );

  /// Add a Phrase to the end of the sequence.
  /// Constructs a new Phrase animating to \a value over \duration from the current end of this Sequence.
  /// Forwards additional arguments to the end of the Phrase constructor.
  ///
  /// Example calls look like:
  /// sequence.then<RampTo>( targetValue, duration, EaseInOutQuad() ).then<Hold>( holdValue, duration );
  template<template <typename> class PhraseT, typename... Args>
  Sequence<T>& then( const T &value, Time duration, Args&&... args );

  /// Clones and appends a phrase to the end of the sequence.
  /// Accepts any concrete Source<T>.
  template<typename PhraseT>
  Sequence<T>& then( const PhraseT &phrase );

  /// Clones and appends a phrase to the end of the sequence.
  /// Specialized to handle shared_ptr's correctly.
  template<typename PhraseT>
  Sequence<T>& then( const std::shared_ptr<PhraseT> &phrase_ptr ) { return then( *phrase_ptr ); }

  /// Clone and append all Phrases from another Sequence to this Sequence.
  Sequence<T>& then( const Sequence<T> &next );

  //
  // Source<T> Overrides.
  //

  /// Returns the Sequence value at \a atTime.
  T getValue( Time atTime ) const override;

  /// Returns the value at the end of the Sequence.
  T getEndValue() const override { return _phrases.empty() ? _initial_value : _phrases.back()->getEndValue(); }

  /// Returns the value at the beginning of the Sequence.
  T getStartValue() const override { return _initial_value; }

  /// Returns a SequenceUniqueRef<T> with copies of all the phrases in this Sequence.
  SourceUniqueRef<T> clone() const override { return SourceUniqueRef<T>( new Sequence<T>( *this ) ); }

  //
  //
  //

  /// Returns the number of phrases in the Sequence.
  size_t getPhraseCount() const { return _phrases.size(); }

  /// Add two sequences together to form a third sequence.
  static Sequence<T> concatenate( Sequence<T> lhs, const Sequence<T> &rhs ) { return lhs.then( rhs ); }

  /// Recursively concatenate any number of sequences.
  template<typename... Args>
  static Sequence<T> concatenate( const Sequence<T> &first, const Sequence<T> &second, Args... additional )
  {
    return concatenate( concatenate( first, second ), std::forward<Args>( additional )... );
  }

private:
  // We store unique pointers to phrases to prevent insanity when copying one sequence into another.
  // We would stack allocate these phrases, but we need pointers to enable polymorphic types.
  std::vector<SourceUniqueRef<T>> _phrases;
  T                               _initial_value;

private:
  void startTimeShifted( Time delta ) override;
};

//=================================================
// Sequence Implementation.
//=================================================

template<typename T>
Sequence<T>& Sequence<T>::set( const T &value )
{
  if( _phrases.empty() ) {
    _initial_value = value;
  }
  else {
    then<Hold>( value, 0.0f );
  }
  return *this;
}

template<typename T>
template<template <typename> class PhraseT, typename... Args>
Sequence<T>& Sequence<T>::then( const T &value, Time duration, Args&&... args )
{
  Time end_time = this->getEndTime() + duration;
  _phrases.emplace_back( std::unique_ptr<PhraseT<T>>( new PhraseT<T>( this->getEndTime(), end_time, this->getEndValue(), value, std::forward<Args>(args)... ) ) );
  this->setEndTime( end_time );

  return *this;
}

template<typename T>
template<typename PhraseT>
Sequence<T>& Sequence<T>::then( const PhraseT &phrase )
{
  std::unique_ptr<Source<T>> p( phrase.clone() );
  Time time = this->getEndTime();
  Time end_time = time + p->getDuration();

  p->setStartTime( time );
  this->setEndTime( end_time );
  _phrases.emplace_back( std::move( p ) );

  return *this;
}

template<typename T>
Sequence<T>& Sequence<T>::then( const Sequence<T> &next )
{
  for( auto &phrase : next._phrases ) {
    then( *phrase );
  }

  return *this;
}

// Would be nice to have a constant-time check (without a while loop).
template<typename T>
T Sequence<T>::getValue( Time atTime ) const
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
    if( (*iter)->getEndTime() >= atTime )
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
void Sequence<T>::startTimeShifted( Time delta )
{
  for( const SourceUniqueRef<T> &phrase : _phrases )
  {
    phrase->shiftTime( delta );
  }
}

//=================================================
// Typedefs and convenience functions.
//=================================================

template<typename T>
using SequenceRef = std::shared_ptr<Sequence<T>>;

template<typename T>
SequenceRef<T> createSequence( T &&initialValue )
{
  return std::make_shared<Sequence<T>>( std::forward<T>( initialValue ) );
}

template<typename T>
SequenceRef<T> createSequence( const T &initialValue )
{
  return std::make_shared<Sequence<T>>( initialValue );
}

} // namespace choreograph
