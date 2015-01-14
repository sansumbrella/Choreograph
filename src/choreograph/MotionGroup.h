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

#include "Timeline.h"

namespace choreograph
{

///
/// MotionGroup composes a Timeline in a TimelineItem.
///
class MotionGroup : public TimelineItem
{
public:
  using Callback = std::function<void (MotionGroup&)>;

  /// Construct with a Timeline that doesn't remove its items on finish.
  MotionGroup();
  /// Construct a motion group from a Timeline
  MotionGroup( Timeline &&timeline );

  MotionGroup( const MotionGroup &rhs ) = delete;

  //====================================
  // TimelineItem Overrides
  //====================================
  void update() final override;
  Time getDuration() const override { return _timeline.getDuration(); }

  /// Returns a reference to the underlying timeline.
  Timeline& timeline() { return _timeline; }

  /// Set a function to be called when we reach the end of the sequence. Receives *this as an argument.
  void setFinishFn( const Callback &c ) { _finish_fn = c; }

  /// Set a function to be called when we start the sequence. Receives *this as an argument.
  void setStartFn( const Callback &c ) { _start_fn = c; }

  /// Set a function to be called when we reach the end of the sequence. Receives *this as an argument.
  void setUpdateFn( const Callback &c ) { _update_fn = c; }

protected:
  void customSetTime( Time time ) final override;
  void customSetPlaybackSpeed( Time s ) final override;

private:
  Timeline  _timeline;

  Callback  _finish_fn;
  Callback  _start_fn;
  Callback  _update_fn;
};

class MotionGroupOptions : public TimelineOptionsBase<MotionGroupOptions>
{
public:
  using Callback = MotionGroup::Callback;

  MotionGroupOptions( MotionGroup &group ):
  TimelineOptionsBase<MotionGroupOptions>( group ),
  _group( group )
  {}

  /// Set function to be called when Motion starts. Receives reference to motion.
  MotionGroupOptions& startFn( const Callback &fn ) { _group.setStartFn( fn ); return *this; }

  /// Set function to be called when Motion updates. Receives current target value.
  MotionGroupOptions& updateFn( const Callback &fn ) { _group.setUpdateFn( fn ); return *this; }

  /// Set function to be called when Motion finishes. Receives reference to motion.
  MotionGroupOptions& finishFn( const Callback &fn ) { _group.setFinishFn( fn ); return *this; }

private:
  MotionGroup &_group;
};

} // namespace choreograph
