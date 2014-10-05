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

namespace choreograph
{
///
/// Combine adds together the value of a collection of other Phrases.
///
template<typename T>
class CombinePhrase : public Phrase<T>
{
public:
  CombinePhrase( Time duration, const PhraseRef<T> &source, float factor=1.0f ):
    Phrase<T>( duration )
  {
    _sources.emplace_back( std::make_pair( source, factor ) );
  }

  CombinePhrase<T>& add( const PhraseRef<T> &source, float factor=1.0f )
  {
    _sources.emplace_back( std::make_pair( source, factor ) );
    return *this;
  }

  template<typename... Args>
  void add( const PhraseRef<T> &source, float factor, Args&&... args )
  {
    add( source, factor );
    add( std::forward<Args>( args )... );
  }

  template<typename... Args>
  static std::shared_ptr<CombinePhrase<T>> create( Time duration, Args&&... args )
  {
    auto phrase = std::shared_ptr<CombinePhrase<T>>( new CombinePhrase<T>( duration ) );
    phrase->add( std::forward<Args>( args )... );
    return phrase;
  }

  T getValue( Time atTime ) const override
  {
    T value = _sources.front().first->getValue( atTime ) * _sources.front().second;
    for( size_t i = 1; i < _sources.size(); ++i ) {
      value += _sources[i].first->getValue( atTime ) * _sources[i].second;
    }
    return value;
  }

  T getStartValue() const override { return getValue( 0 ); }
  T getEndValue() const override { return getValue( this->getDuration() ); }

private:
  CombinePhrase( Time duration ):
    Phrase<T>( duration )
  {}
  // Collection of shared_ptr, mix pairs.
  // shared_ptr since unique_ptr made copying/moving std::pair impossible
  std::vector<std::pair<PhraseRef<T>, float>>  _sources;
};

/// Free function to make combining phrases easier in client code.
template<typename T, typename... Args>
std::shared_ptr<CombinePhrase<T>> sumPhrases( Time duration, const PhraseRef<T> &phrase_a, float mix_a, const PhraseRef<T> &phrase_b, float mix_b, Args&&... args )
{
  return CombinePhrase<T>::create( duration, phrase_a, mix_a, phrase_b, mix_b, std::forward<Args>( args )... );
}

///
/// Mix interpolates between the value of two input Phrases.
/// Untested.
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

  static std::shared_ptr<MixPhrase<T>> create( const PhraseRef<T> &a, const PhraseRef<T> &b, float mix = 0.5f ) {
    return std::make_shared<MixPhrase<T>>( a, b, mix );
  }

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
