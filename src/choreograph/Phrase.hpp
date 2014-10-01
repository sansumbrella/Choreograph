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

// Includes needed for tests to build (since they don't include the pch).
#include <functional>
#include <vector>
#include <cmath>
#include <memory>

namespace choreograph
{

/// Define our Time type here so it's easier to change out if needed.
/// Float loses precision pretty quickly, but is fast and doesn't take up much space.
/// Double works nicely when adding together lots of little bits or if you need long sequences.
using Time = float;

template<typename T>
class Sequence;

///
/// A Phrase of motion.
/// Virtual base class with concept of value and implementation of time.
///
template<typename T>
class Phrase
{
public:
  Phrase() = default;

  Phrase( Time duration ):
    _duration( duration )
  {}

  virtual ~Phrase() = default;

  /// Override to provide value at requested time.
  /// Returns the interpolated value at the given time.
  virtual T getValue( Time atTime ) const = 0;

  /// Override to provide value at start (and before).
  virtual T getStartValue() const = 0;

  /// Override to provide value at end (and beyond).
  virtual T getEndValue() const = 0;

  /// Override to provide a copy of the derived Phrase. Needed for phrases to be copy-composable.
  virtual std::unique_ptr<Phrase<T>> clone() const = 0;

  /// Returns the Phrase value at \a time, looping past the end from inflection point to the end.
  /// Relies on the subclass implementation of getValue( t ).
  T getValueWrapped( Time time, Time inflectionPoint = 0.0f ) const { return getValue( wrapTime( time, inflectionPoint ) ); }

  /// Returns normalized time if t is in range [start_time, end_time].
  inline Time normalizeTime( Time t ) const { return t / _duration; }
  /// Returns the duration of this source.
  inline Time getDuration() const { return _duration; }

  /// Wrap \a time around \a inflectionPoint in the Sequence.
  Time wrapTime( Time time, Time inflectionPoint = 0.0f ) const
  {
    if( time > getDuration() ) {
      return inflectionPoint + std::fmodf( time, getDuration() - inflectionPoint );
    }
    else {
      return time;
    }
  }

private:
  Time _duration = 0;

  /// Sets the duration of this source.
  /// Generally, create a new source with the duration you want and use that.
  inline void setDuration( Time duration ) { _duration = duration; }

  friend class Sequence<T>;
};

template<typename T>
using PhraseRef = std::shared_ptr<Phrase<T>>;

template<typename T>
using PhraseUniqueRef = std::unique_ptr<Phrase<T>>;

} // namespace choreograph
