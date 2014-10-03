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

#include "Phrase.hpp"

namespace choreograph
{
/// EaseFn's describes a one-dimensional transformation through time.
/// Choreograph accepts any function taking and returning a normalized float.
/// For a large number of ease functions, see Cinder's Easing.h
/// Generally, it is assumed that the following holds true for an EaseFn:
/// EaseFn( 0 ) = 0, EaseFn( 1 ) = 1.
typedef std::function<float (float)> EaseFn;


/**
 \file
 Phrases for use in Choreograph.
 A Phrase is a part of a Sequence.
 It describes the motion between two positions.
 This is the essence of a Tween, with all values held internally.
*/

namespace
{
/// Default ease function for ramps.
inline float easeNone( float t ) { return t; }

} // namespace

/// The default templated lerp function.
template<typename T>
T lerpT( const T &a, const T &b, float t )
{
  return a + (b - a) * t;
}

//=================================================
// Basic Phrases.
//=================================================

///
/// RampTo is a phrase that interpolates all components with an ease function.
///
template<typename T>
class RampTo : public Phrase<T>
{
public:
  using LerpFn = std::function<T (const T&, const T&, float)>;

  RampTo( Time duration, const T &start_value, const T &end_value, const EaseFn &ease_fn = &easeNone, const LerpFn &lerp_fn = &lerpT<T> ):
    Phrase<T>( duration ),
    _start_value( start_value ),
    _end_value( end_value ),
    _easeFn( ease_fn ),
    _lerpFn( lerp_fn )
  {}

  /// Returns the interpolated value at the given time.
  T getValue( Time atTime ) const override
  {
    return _lerpFn( _start_value, _end_value, _easeFn( this->normalizeTime( atTime ) ) );
  }

  T getStartValue() const override { return _start_value; }

  T getEndValue() const override { return _end_value; }

private:
  T       _start_value;
  T       _end_value;
  EaseFn  _easeFn;
  LerpFn  _lerpFn;
};


///
/// RampToN is a phrase template with N separately-interpolated components.
/// Allows for the use of separate ease functions per component.
/// All components must be of the same type.
/// If fewer than N ease functions are provided, the last ease function will be used to fill out remaining components.
/// If you only want one ease function for all components, use a normal RampTo.
///
template<unsigned int SIZE, typename T>
class RampToN : public Phrase<T>
{

public:

  template<typename... Args>
  RampToN( Time duration, const T &start_value, const T &end_value, Args&&... args ):
    Phrase<T>( duration ),
    _start_value( start_value ),
    _end_value( end_value )
  {
    std::vector<EaseFn> list = { std::forward<Args>( args )... };
    for( size_t i = 0; i < SIZE && i < list.size(); ++i ) {
      _ease_fns[i] = list[i];
    }
    for( size_t i = list.size(); i < SIZE; ++i ) {
      _ease_fns[i] = list.back();
    }
  }

  /// Returns the interpolated value at the given time.
  T getValue( Time atTime ) const override
  {
    Time t = this->normalizeTime( atTime );
    T out;
    for( int i = 0; i < SIZE; ++i )
    {
      out[i] = _componentLerpFn( _start_value[i], _end_value[i], _ease_fns[i]( t ) );
    }
    return out;
  }

  T getStartValue() const override { return _start_value; }
  T getEndValue() const override { return _end_value; }

private:
  using ComponentT = decltype( T().x ); // get the type of the x component. decltype( T()[0] ) doesn't compile with glm's vecN unions.
  using ComponentLerpFn = std::function<ComponentT (const ComponentT&, const ComponentT&, float)>;

  ComponentLerpFn           _componentLerpFn = &lerpT<ComponentT>;
  T                         _start_value;
  T                         _end_value;
  std::array<EaseFn, SIZE>  _ease_fns;
};

/// RampTo2 is a phrase with 2 separately-interpolated components.
template<typename T>
using RampTo2 = RampToN<2, T>;

/// RampTo3 is a phrase with 3 separately-interpolated components.
template<typename T>
using RampTo3 = RampToN<3, T>;

/// RampTo4 is a phrase with 4 separately-interpolated components.
template<typename T>
using RampTo4 = RampToN<4, T>;

///
/// Hold is a phrase that hangs in there, never changing.
///
template<typename T>
class Hold : public Phrase<T>
{
public:

  Hold( Time duration, const T &start_value, const T &end_value ):
  Phrase<T>( duration ),
  _value( end_value )
  {}

  T getValue( Time atTime ) const override
  {
    return _value;
  }

  T getStartValue() const override
  {
    return _value;
  }

  T getEndValue() const override
  {
    return _value;
  }

private:
  T       _value;
};

//=================================================
// Phrase Decorators. Meta Phrases.
//=================================================

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
std::shared_ptr<CombinePhrase<T>> combinePhrases( Time duration, const PhraseRef<T> &phrase_a, float mix_a, const PhraseRef<T> &phrase_b, float mix_b, Args&&... args )
{
  return CombinePhrase<T>::create( duration, phrase_a, mix_a, phrase_b, mix_b, std::forward<Args>( args )... );
}

template<typename T>
class LoopPhrase : public Phrase<T>
{
public:
  /// Create a Phrase that loops \a source \a numLoops times.
  LoopPhrase( const PhraseRef<T> &source, float numLoops, Time inflectionPoint = 0.0f ):
    Phrase<T>( source->getDuration() * numLoops ),
    _source( source ),
    _inflection_point( inflectionPoint )
  {}

  static std::shared_ptr<LoopPhrase<T>> create( const PhraseRef<T> &source, float numLoops, Time inflectionPoint = 0.0f )
  {
    return std::make_shared<LoopPhrase<T>>( source, numLoops, inflectionPoint );
  }

  T getValue( Time atTime ) const override { return _source->getValueWrapped( atTime, _inflection_point ); }
  T getStartValue() const override { return _source->getStartValue(); }
  T getEndValue() const override { return _source->getEndValue(); }
private:
  PhraseRef<T>  _source;
  Time          _inflection_point;
};

/// Free function to make creating looped phrases easier in client code.
template<typename T>
PhraseRef<T> loopPhrase( const PhraseRef<T> &source, float numLoops, Time inflectionPoint = 0.0f )
{
  return LoopPhrase<T>::create( source, numLoops, inflectionPoint );
}

//=================================================
// Weirder Phrases.
//=================================================

/**
 AnalyticChange is a phrase that calls a std::function every step.
 You could do similar things with a Motion's updateFn, but this is composable within Sequences.
 */
template<typename T>
class AnalyticChange : public Phrase<T>
{
public:
  /// Analytic Function receives start, end, and normalized time.
  /// Most basic would be mix( a, b, t ) or lerp( a, b, t ).
  /// Intended use is to apply something like cos() or random jitter.
  using Function = std::function<T (const T& startValue, const T& endValue, Time normalizedTime, Time duration)>;

  AnalyticChange( Time duration, const T &start_value, const T &end_value, const Function &fn ):
    Phrase<T>( duration ),
    _function( fn ),
    _start_value( start_value ),
    _end_value( end_value )
  {}

  T getValue( Time atTime ) const override
  {
    return _function( _start_value, _end_value, this->normalizeTime( atTime ), this->getDuration() );
  }

  T getStartValue() const override { return _start_value; }
  T getEndValue() const override { return _end_value; }

private:
  Function  _function;
  T         _start_value;
  T         _end_value;
};

} // namespace atlantic
