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

#include "Sequence.hpp"
#include "Output.h"

namespace choreograph
{

class OutputBase;

/**
A connection between a continuous, independent Sequence and an output.
Non-templated base type so we can store in a polymorphic container.
*/
class MotionBase
{
public:
  explicit MotionBase( void *target );

  MotionBase( OutputBase *base );

	virtual ~MotionBase();

	//! Move motion forward in time.
	void step( float dt );

  //! Overridden to determine what a time step does.
	virtual void update() = 0;

  //! Returns the duration of the motion.
	virtual float getDuration() const = 0;

	//! Returns current animation time in seconds.
	float time() const { return _time - _start_time; }

	//! Returns previous step's animation time in seconds.
	float previousTime() const { return _previous_time - _start_time; }

	//! Returns true if animation plays forward with positive time steps.
	bool  forward() const { return _speed >= 0.0f; }

	//! Returns true if animation plays backward with positive time steps.
	bool  backward() const { return _speed < 0.0f; }

  //! Returns true if this Motion's time is past the end of its duration. Accounts for reversed playback.
  bool  isFinished() const;

  //! Set playback speed of motion. Use negative numbers to play in reverse.
	void  setPlaybackSpeed( float s ) { _speed = s; }

  //! Returns the current playback speed of motion.
	float getPlaybackSpeed() const { return _speed; }

  //! Reset motion to beginning. Accounts for reversed playback.
  void  resetTime();

	//! Returns true if this Motion has an output.
	bool  isValid() const { return _target != nullptr; }

	//! Returns true if this Motion has no output.
	bool  isInvalid() const { return _target == nullptr; }

protected:
	// True if the underlying Sequence should play forever.
	bool        _continuous = false;

private:

	// Null pointer to target, used for comparison with other MotionBase's.
	void        *_target = nullptr;
  // Pointer to safe handle type. Exists if created with an Output<T> target.
  OutputBase  *_output_base = nullptr;

	//! Playback speed. Set to negative to go in reverse.
	float       _speed = 1.0f;
	//! Current animation time in seconds. Time at which Sequence is evaluated.
	float       _time = 0.0f;
	//! Previous animation time in seconds.
	float       _previous_time = 0.0f;
	//! Animation start time in seconds. Time from which Sequence is evaluated.
  //! Use to apply a delay.
	float       _start_time = 0.0f;

	//! Called on destruction of either MotionBase or _output_base.
  void disconnect();

  friend class OutputBase;
  friend class Timeline;
};

class Cue : public MotionBase
{
public:
  //! Calls cue function if time threshold has been crossed.
	void update() override;

  //! Cues are instantaneous.
  float getDuration() const override { return 0.0f; }
private:
	std::function<void ()> _cue;
};

/**
Motion: a sequence embedded in time and connected to an output.
A connection between a continuous, independent Sequence and an output.
Drives a Sequence and sends its value to a user-defined variable.
Might mirror the Sequence interface for easier animation.
*/
template<typename T>
class Motion : public MotionBase
{
public:
  Motion() = delete;

  explicit Motion( T *target ):
    MotionBase( target ),
    _output( target )
  {
    ci::app::console() << "Motion from T*" << std::endl;
  }

  explicit Motion( Output<T> *target ):
    MotionBase( target ),
    _output( target->ptr() )
  {
    ci::app::console() << "Motion from Output<T>*" << std::endl;
  }


  //! Duration is based on the underlying sequence.
	float getDuration() const override { return _sequence->getDuration(); }

	float getProgress() const { return time() / _sequence->getDuration(); }

	//! Returns the underlying sequence for extension.
	Sequence<T>&  getSequence() { return *_sequence; }

	typedef std::function<void (const T&)>        DataCallback;
	typedef std::function<void (Motion<T> &)> Callback;
	//! Set a function to be called when we reach the end of the sequence.
	Motion<T>&  finishFn( const Callback &c ) { _finishFn = c; return *this; }
	//! Set a function to be called when we start the sequence.
	Motion<T>&  startFn( const Callback &c ) { _startFn = c; return *this; }
	//! Set a function to be called at each update step of the sequence. Called immediately after setting the target value.
	Motion<T>&  updateFn( const DataCallback &c ) { _updateFn = c; return *this; }

	Motion<T>&  playbackSpeed( float s ) { setPlaybackSpeed( s ); return *this; }

	//! Set the connection to play continuously.
	Motion<T>&  continuous( bool c ) { _continuous = c; return *this; }

  //! Update
  void update() override
	{
    assert( isValid() );

		if( _startFn ) {
			if( forward() && time() > 0.0f && previousTime() <= 0.0f )
				_startFn( *this );
			else if( backward() && time() < _sequence->getDuration() && previousTime() >= _sequence->getDuration() )
				_startFn( *this );
		}

		*_output = _sequence->getValue( time() );
		if( _updateFn ) {
			_updateFn( *_output );
		}

		if( _finishFn ){
			if( forward() && time() >= _sequence->getDuration() && previousTime() < _sequence->getDuration() )
      {
				_finishFn( *this );
      }
			else if( backward() && time() <= 0.0f && previousTime() > 0.0f )
      {
				_finishFn( *this );
      }
		}
	}

private:
	// shared_ptr to sequence since many connections could share the same sequence
	// this enables us to do pseudo-instancing on our animations, reducing their memory footprint.
	std::shared_ptr<Sequence<T>> _sequence;
	T             *_output = nullptr;
	Callback      _finishFn = nullptr;
	Callback      _startFn = nullptr;
	DataCallback  _updateFn = nullptr;

  friend class Timeline;
};

} // namespace choreograph
