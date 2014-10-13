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

#include "TimeType.h"

namespace choreograph
{

using TimelineItemRef = std::shared_ptr<class TimelineItem>;

using TimelineItemUniqueRef = std::unique_ptr<class TimelineItem>;

///
/// TimelineItem: non-templated base for polymorphic Motions.
/// Base class for anything that can go on a Timeline.
///
class TimelineItem
{
public:
  TimelineItem() = default;

  virtual ~TimelineItem() = default;

  //=================================================
  // Common public interface.
  //=================================================

  /// Advance motion in time. Affected by Motion's speed. Do not use from callbacks (it will fire them).
  void step( Time dt );

  /// Jump to a point in time. Ignores Motion's speed. Do not use from callbacks (it will fire them).
  void jumpTo( Time time );

  /// Set time. Ignores speed. Safe to use from callbacks.
  void setTime( Time time ) { _time = _previous_time = time; }

  //=================================================
  // Virtual Interface.
  //=================================================

  /// Overridden to determine what a time step does.
  virtual void update() = 0;

  /// Returns the duration of the motion.
  virtual Time getDuration() const = 0;

  /// Returns true iff motion is no longer valid.
  virtual bool isInvalid() const { return false; }

  /// Returns target if motion has one.
  virtual const void* getTarget() const { return nullptr; }

  //=================================================
  // Time manipulation and querying.
  //=================================================

  /// Returns current animation time in seconds.
  Time time() const { return _time - _start_time; }

  /// Returns previous step's animation time in seconds.
  Time previousTime() const { return _previous_time - _start_time; }

  /// Returns true if animation plays forward with positive time steps.
  bool  forward() const { return _speed >= 0.0f; }

  /// Returns true if animation plays backward with positive time steps.
  bool  backward() const { return _speed < 0.0f; }

  /// Returns true if this Motion's time is past the end of its duration. Accounts for reversed playback.
  bool  isFinished() const;

  /// Set playback speed of motion. Use negative numbers to play in reverse.
  void  setPlaybackSpeed( Time s ) { _speed = s; }

  /// Returns the current playback speed of motion.
  Time getPlaybackSpeed() const { return _speed; }

  /// Reset motion to beginning. Accounts for reversed playback.
  void resetTime();

  /// Returns the current end time of this motion.
  Time getEndTime() const { return getStartTime() + getDuration(); }

  /// Set the start time of this motion. Use to delay entire motion.
  void setStartTime( Time t ) { _start_time = t; }
  Time getStartTime() const { return _start_time; }

  /// Set whether the Motion should be removed from parent Timeline on finish.
  void setRemoveOnFinish( bool doRemove ) { _remove_on_finish = doRemove; }

  /// Returns true if the Motion should be removed from parent Timeline on finish.
  bool getRemoveOnFinish() const { return _remove_on_finish; }

private:
  /// True if this motion should be removed from Timeline on finish.
  bool       _remove_on_finish = true;
  /// Playback speed. Set to negative to go in reverse.
  Time       _speed = 1;
  /// Current animation time in seconds. Time at which Sequence is evaluated.
  Time       _time = 0;
  /// Previous animation time in seconds.
  Time       _previous_time = 0;
  /// Animation start time in seconds. Time from which Sequence is evaluated.
  /// Use to apply a delay.
  Time       _start_time = 0;
};

} // namespace choreograph
