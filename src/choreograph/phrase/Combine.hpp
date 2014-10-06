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

#include "choreograph/Phrase.hpp"

///
/// \file
/// Combine Phrases take multiple other Phrases and combine them into
/// a single output value. Unlike Sequences, Combine Phrases consider
/// their input Phrases as coincident in time.
///

namespace choreograph
{
///
/// Mix interpolates between the value of two input Phrases.
///
template<typename T>
class MixPhrase : public Phrase<T>
{
public:
  MixPhrase( const PhraseRef<T> &a, const PhraseRef<T> &b, float mix = 0.5f ):
    Phrase<T>( std::max( a->getDuration(), b->getDuration() ) ),
    _a( a ),
    _b( b ),
    _mix( mix )
  {}

  /// Returns a blend of the values of a and b at \a atTime.
  T getValue( Time atTime ) const override {
    return _a->getValue( atTime ) * mixA() + _b->getValue( atTime ) * mixB();
  }

  T getStartValue() const override {
    return _a->getStartValue() * mixA() + _b->getStartValue() * mixB();
  }

  T getEndValue() const override {
    return _a->getEndValue() * mixA() + _b->getEndValue() * mixB();
  }

  /// Sets the balance of the Phrase mix. Values should be in the range [0, 1].
  void setMix( float amount ) { _mix = amount; }

  /// Returns the current balance of the Phrase mix.
  float getMix() const { return _mix; }

  /// Returns a pointer to the mix output for animation with a choreograph::Motion.
  Output<float>* getMixOutput() { return &_mix; }

private:
  Output<float> _mix = 0.5f;
  PhraseRef<T>  _a;
  PhraseRef<T>  _b;

  inline float mixA() const { return 1 - _mix(); }
  inline float mixB() const { return _mix(); }
};

///
/// AccumulatePhrase adds together the value of a collection of other Phrases.
/// Performs the functional operation reduce or foldl over the Phrases in question.
/// By default, sums the value of all Phrases.
///
template<typename T>
class AccumulatePhrase : public Phrase<T>
{
public:
  using ReduceFunction = std::function<T (const T&, const T&)>;

  AccumulatePhrase( const T &initial_value, const PhraseRef<T> &a, const PhraseRef<T> &b, const ReduceFunction &fn ):
    Phrase<T>( std::max( a->getDuration(), b->getDuration() ) ),
    _reduceFn( fn ),
    _initial_value( initial_value )
  {
    add( a, b );
  }

  AccumulatePhrase( const T &initial_value, Time duration, const PhraseRef<T> &a, const PhraseRef<T> &b, const ReduceFunction &fn ):
    Phrase<T>( duration ),
    _reduceFn( fn ),
    _initial_value( initial_value )
  {
    add( a, b );
  }

  /// Add additional Phrase to be summed.
  void add( const PhraseRef<T> &source )
  {
    _sources.push_back( source );
  }

  /// Add additional Phrases to be summed.
  template<typename... Args>
  void add( const PhraseRef<T> &source, Args&&... args )
  {
    add( source );
    add( std::forward<Args>( args )... );
  }

  T getValue( Time atTime ) const override
  {
    T value = _initial_value;
    for( const auto &source : _sources ) {
      value = _reduceFn( value, source->getValue( atTime ) );
    }
    return value;
  }

  T getStartValue() const override { return getValue( 0 ); }
  T getEndValue() const override { return getValue( this->getDuration() ); }

  /// Default reduce function sums all inputs.
  static T sum( const T &a, const T &b ) {
    return a + b;
  }

private:
  // Reduce function to apply.
  ReduceFunction            _reduceFn;
  // Phrases to reduce.
  std::vector<PhraseRef<T>> _sources;
  T                         _initial_value;
};

///
/// Raise combines one-dimensional phrases into a higher-dimensional phrase.
/// This allows you to use two float Phrases as the x and y channels of a vec2 phrase, for example.
/// Untested.
///
template<typename T>
class RaisePhrase : public Phrase<T>
{
  using ComponentT = decltype( T().x ); // get the type of the x component. decltype( T()[0] ) doesn't compile with glm's vecN unions.

  template<typename... Args>
  RaisePhrase( Time duration, Args&&... args ):
    Phrase<T>( duration ),
    _sources( std::forward<Args>( args )... )
  {}

  T getStartValue() const override
  {
    T out;
    for( size_t i = 0; i < _sources.size(); ++i ) {
      out[i] = _sources[i]->getStartValue();
    }
    return out;
  }

  T getValue( float atTime ) const override
  {
    T out;
    for( size_t i = 0; i < _sources.size(); ++i ) {
      out[i] = _sources[i]->getValue( atTime );
    }
    return out;
  }

  T getEndValue() const override
  {
    T out;
    for( size_t i = 0; i < _sources.size(); ++i ) {
      out[i] = _sources[i]->getEndValue();
    }
    return out;
  }

private:
  std::vector<PhraseRef<ComponentT>> _sources;
};


} // namespace choreograph
