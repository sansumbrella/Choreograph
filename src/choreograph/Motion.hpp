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

/**
A connection between a continuous, independent Sequence and an output.
Non-templated base type so we can store in a polymorphic container.
*/
class MotionBase
{
public:
	virtual ~MotionBase() = default;

	//! Move forward in time.
	void step( float dt )
	{
		_time += dt * _speed;
		update();
		_previous_time = _time;
	}
	virtual void update() = 0;

	virtual float getDuration() const = 0;

	//! Returns current animation time in seconds.
	float time() const { return _time - _start_time; }
	//! Returns previous step's animation time in seconds.
	float previousTime() const { return _previous_time - _start_time; }

	//! Returns true if animation plays forward with positive time steps.
	bool forward() const { return _speed > 0.0f; }
	//! Returns true if animation plays backward with positive time steps.
	bool backward() const { return _speed < 0.0f; }

	// True if the underlying Sequence should play forever.
	bool        _continuous = false;

	void        *_target = nullptr;

	void        setPlaybackSpeed( float s ) { _speed = s; }
	float       getPlaybackSpeed() const { return _speed; }

private:
	//! Playback speed. Set to negative to go in reverse.
	float       _speed = 1.0f;
	//! Current animation time in seconds. Time at which Sequence is evaluated.
	float       _time = 0.0f;
	//! Previous animation time in seconds.
	float       _previous_time = 0.0f;
	//! Animation start time in seconds. Time from which Sequence is evaluated.
	float       _start_time = 0.0f;
};

class Cue : public MotionBase
{
public:
	void update() override
	{
		if( forward() && time() >= 0.0f && previousTime() < 0.0f )
			_cue();
		else if( forward() && time() <= 0.0f && previousTime() > 0.0f )
			_cue();
	}
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
	void update() override
	{
		if( isValid() )
		{
			if( _startFn ) {
				if( forward() && time() > 0.0f && previousTime() <= 0.0f )
					_startFn( *this );
				else if( backward() && time() < sequence->getDuration() && previousTime() >= sequence->getDuration() )
					_startFn( *this );
			}

			*output = sequence->getValue( time() );
			if( _updateFn ) {
				_updateFn( *output );
			}

			if( _finishFn ){
				if( forward() && time() >= sequence->getDuration() && previousTime() < sequence->getDuration() )
					_finishFn( *this );
				else if( backward() && time() <= 0.0f && previousTime() > 0.0f )
					_finishFn( *this );
			}
		}
	}

	//! Returns true if this Motion has no output.
	bool isInvalid() const { return output == nullptr; }
	//! Returns true if this Motion has an output.
	bool isValid() const { return output != nullptr; }

	float getDuration() const override { return sequence->getDuration(); }

	float getProgress() const { return time() / sequence->getDuration(); }

	//! Returns the underlying sequence for extension.
	Sequence<T>&  getSequence() { return *sequence; }

	typedef std::function<void (const T&)>        DataCallback;
	typedef std::function<void (Motion<T> &)> Callback;
	//! Set a function to be called when we reach the end of the sequence.
	Motion<T>& finishFn( const Callback &c ) { _finishFn = c; return *this; }
	//! Set a function to be called when we start the sequence.
	Motion<T>& startFn( const Callback &c ) { _startFn = c; return *this; }
	//! Set a function to be called at each update step of the sequence. Called immediately after setting the target value.
	Motion<T>& updateFn( const DataCallback &c ) { _updateFn = c; return *this; }

	Motion<T>&    playbackSpeed( float s ) { setPlaybackSpeed( s ); return *this; }

	//! Set the connection to play continuously.
	Motion<T>& continuous( bool c ) { _continuous = c; return *this; }

	// shared_ptr to sequence since many connections could share the same sequence
	// this enables us to do pseudo-instancing on our animations, reducing their memory footprint.
	std::shared_ptr<Sequence<T>> sequence;
	T             *output = nullptr;
	Callback      _finishFn = nullptr;
	Callback      _startFn = nullptr;
	DataCallback  _updateFn = nullptr;
};

} // namespace choreograph
