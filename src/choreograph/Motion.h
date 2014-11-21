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

#include "TimelineItem.h"
#include "Sequence.hpp"
#include "Connection.hpp"
#include "Output.hpp"

namespace choreograph
{

//=================================================
// Aliases.
//=================================================

template<typename T>
class Motion;

template<typename T>
using MotionRef = std::shared_ptr<Motion<T>>;


template<typename T>
struct MotionGroupOptions
{
  explicit MotionGroupOptions( Motion<T> &motion ):
    _motion( motion )
  {}

  /// Set a function to be called when we start the motion. Receives Motion as an argument.
  MotionGroupOptions& startFn( const typename Motion<T>::Callback &fn ) { _motion.setStartFn( fn ); return *this; }

  /// Set a function to be called on Motion update. Receives target as an argument.
  MotionGroupOptions& updateFn( const typename Motion<T>::DataCallback &fn ) { _motion.setUpdateFn( fn ); return *this; }

  /// Set a function to be called when we reach the end of the motion. Receives Motion as an argument.
  MotionGroupOptions& finishFn( const typename Motion<T>::Callback &fn ) { _motion.setFinishFn( fn ); return *this; }

private:
  Motion<T> &_motion;
};
///
/// Groups together a number of Motions so they can be repeated
/// and moved around together.
/// Note that grouped Motions all share the group's speed and time.
/// Assumes that the underlying Sequences won't change after adding.
///
class MotionGroup : public TimelineItem
{
public:
  using Callback = std::function<void (MotionGroup&)>;

  static std::unique_ptr<MotionGroup> create() { return std::unique_ptr<MotionGroup>( new MotionGroup ); }

  /// Create and add a Motion to the group.
  /// The Motion will apply \a sequence to \a output.
  template<typename T>
  MotionGroupOptions<T> add( const SequenceRef<T> &sequence, Output<T> *output );

  /// Create and add a Motion to the group.
  /// The Phrase is converted to a Sequence.
  template<typename T>
  MotionGroupOptions<T> add( const PhraseRef<T> &phrase, Output<T> *output ) { return add( std::make_shared<Sequence<T>>( phrase ), output ); }

  /// Update all grouped motions.
  void update() override;

  /// Returns true if any grouped motions are invalid.
  bool isInvalid() const override;

  /// Returns the duration of the motion group (end time of last motion).
  Time getDuration() const override { return _duration; }

  void setFinishFn( const Callback &fn ) { _finish_fn = fn; }
  void setStartFn( const Callback &fn ) { _start_fn = fn; }

  void extendDuration( Time amount ) { _duration += amount; }

private:
  MotionGroup() = default;
  Time                                _duration = 0;
  std::vector<TimelineItemUniqueRef>  _motions;

  Callback                            _start_fn = nullptr;
  Callback                            _finish_fn = nullptr;
};

///
/// Motion: Moves a playhead along a Sequence and sends its value to a user-defined output.
/// Connects a Sequence and an Output.
///
template<typename T>
class Motion : public TimelineItem
{
public:
  using MotionT       = Motion<T>;
  using SequenceRefT  = SequenceRef<T>;
  using DataCallback  = std::function<void (T&)>;
  using Callback      = std::function<void (MotionT&)>;

  Motion() = delete;

  Motion( T *target, const SequenceRefT &sequence ):
    _connection( target ),
    _source( sequence )
  {}

  Motion( Output<T> *target, const SequenceRefT &sequence ):
    _connection( target ),
    _source( sequence )
  {}

  /// Returns duration of the underlying sequence.
  Time getDuration() const override { return _source->getDuration(); }

  /// Returns ratio of time elapsed, from [0,1] over duration.
  Time getProgress() const { return time() / _source->getDuration(); }

  /// Returns the underlying Sequence sampled for this motion.
  SequenceRefT  getSequence() { return _source; }

  inline bool isInvalid() const override { return _connection.isDisconnected(); }
  const void* getTarget() const override { return _connection.targetPtr(); }

  /// Set a function to be called when we reach the end of the sequence. Receives *this as an argument.
  void setFinishFn( const Callback &c ) { _finishFn = c; }

  /// Set a function to be called when we start the sequence. Receives *this as an argument.
  void setStartFn( const Callback &c ) { _startFn = c; }

  /// Set a function to be called when we cross the given inflection point. Receives *this as an argument.
  void addInflectionCallback( size_t inflection_point, const Callback &callback );

  /// Set a function to be called at each update step of the sequence.
  /// Function will be called immediately after setting the target value.
  void setUpdateFn( const DataCallback &c ) { _updateFn = c; }

  /// Update the connected target with the current sequence value.
  /// Calls start/update/finish functions as appropriate if assigned.
  void update() override;

  /// Removes elements from sequence before specified time.
  /// Don't use this if the Motion shares a sequence with others.
  /// Note that you can safely share sequences if you add them to each motion as phrases.
  void burnTracksBefore( Time time );

private:
  // shared_ptr to source since many connections could share the same source.
  // This enables us to do pseudo-instancing on our animations, reducing their memory footprint.
  // Might switch to a named sequence object rather than a shared_ptr in future.
  SequenceRefT    _source;
  Connection<T>   _connection;

  Callback        _finishFn = nullptr;
  Callback        _startFn  = nullptr;
  DataCallback    _updateFn = nullptr;
  std::vector<std::pair<size_t, Callback>>  _inflectionCallbacks;
};

//=================================================
// MotionGroup Template Implementation.
//=================================================

template<typename T>
MotionGroupOptions<T> MotionGroup::add( const SequenceRef<T> &sequence, Output<T> *output )
{
  auto motion = std::make_unique<Motion<T>>( output, sequence );
  auto motion_ptr = motion.get();


  _motions.emplace_back( std::move( motion ) );

  _duration = std::max( _duration, motion_ptr->getDuration() );

  return MotionGroupOptions<T>( *motion_ptr );
}

//=================================================
// Motion Template Implementation.
//=================================================

template<typename T>
void Motion<T>::update()
{
  if( Motion<T>::isInvalid() ) {
    return;
  }

  if( _startFn )
  {
    if( forward() && time() > 0.0f && previousTime() <= 0.0f ) {
      _startFn( *this );
    }
    else if( backward() && time() < getDuration() && previousTime() >= getDuration() ) {
      _startFn( *this );
    }
  }

  _connection.target() = _source->getValue( time() );

  if( ! _inflectionCallbacks.empty() )
  {
    auto points = _source->getInflectionPoints( previousTime(), time() );
    if( points.first != points.second ) {
      // We just crossed an inflection point...
      auto crossed = std::max( points.first, points.second );
      for( const auto &fn : _inflectionCallbacks ) {
        if( fn.first == crossed ) {
          fn.second( *this );
        }
      }
    }
  }

  if( _updateFn )
  {
    _updateFn( _connection.target() );
  }

  if( _finishFn )
  {
    if( forward() && time() >= getDuration() && previousTime() < getDuration() ) {
      _finishFn( *this );
    }
    else if( backward() && time() <= 0.0f && previousTime() > 0.0f ) {
      _finishFn( *this );
    }
  }
}

template<typename T>
void Motion<T>::addInflectionCallback( size_t inflection_point, const Callback &callback )
{
  _inflectionCallbacks.emplace_back( std::make_pair( inflection_point, callback ) );
}

template<typename T>
void Motion<T>::burnTracksBefore( Time time )
{
  _source = std::make_shared<Sequence>( _source->slice( 0, time ) );
}

} // namespace choreograph
