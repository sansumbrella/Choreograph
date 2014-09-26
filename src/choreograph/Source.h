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

namespace choreograph
{

///
/// A Source of motion.
/// Virtual base class with concept of value and implementation of time.
///
template<typename T>
class Source
{
public:
  Source() = default;

  Source( float startTime, float endTime ):
    _start_time( startTime ),
    _end_time( endTime )
  {}

  virtual ~Source() = default;

  /// Override to provide value at requested time.
  /// Returns the interpolated value at the given time.
  virtual T getValue( float atTime ) const = 0;

  /// Override to provide value at start (and before).
  virtual T getStartValue() const = 0;

  /// Override to provide value at end (and beyond).
  virtual T getEndValue() const = 0;

  /// Returns the Source value at \a time, looping past the end from inflection point to the end.
  /// Relies on the subclass implementation of getValue( t ).
  T getValueWrapped( float time, float inflectionPoint = 0.0f ) const { return getValue( wrapTime( time, inflectionPoint ) ); }

  /// Returns normalized time if t is in range [start_time, end_time].
  inline float normalizeTime( float t ) const { return (t - _start_time) / (_end_time - _start_time); }
  /// Returns the start time of this source.
  inline float getStartTime() const { return _start_time; }
  /// Returns the end time of this source.
  inline float getEndTime() const { return _end_time; }
  /// Returns the duration of this source.
  inline float getDuration() const { return _end_time - _start_time; }

  /// Wrap \a time around \a inflectionPoint in the Sequence.
  float wrapTime( float time, float inflectionPoint = 0.0f ) const
  {
    if( time > getDuration() ) {
      return inflectionPoint + std::fmodf( time, getDuration() - inflectionPoint );
    }
    else {
      return time;
    }
  }
  /// Offsets start and end times.
  void shiftTime( float amount ) { _start_time += amount; _end_time += amount; startTimeShifted( amount ); }

protected:
  /// Sets end time.
  void setEndTime( float t ) { _end_time = t; }
  /// Sets start time while preserving duration.
  void setStartTime( float t ) { float delta = t - _start_time; _start_time += delta; _end_time += delta; startTimeShifted( delta ); }

  /// Override to handle special cases when time changes.
  /// Needed by Sequence so it can shift all its child sources.
  virtual void startTimeShifted( float delta ) {}

private:
  float _start_time = 0.0f;
  float _end_time = 0.0f;
};

template<typename T>
class SourceT
{

};

template<typename T>
using SourceRef = std::shared_ptr<Source<T>>;

template<typename T>
using SourceUniqueRef = std::unique_ptr<Source<T>>;

} // namespace choreograph
