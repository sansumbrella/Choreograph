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

#include "Motion.h"

#include "Output.h"

using namespace choreograph;
using namespace std;
using namespace cinder;

//
//  MotionBase
//

MotionBase::MotionBase( void *target ):
  _target( target )
{
  app::console() << "MotionBase from void*" << endl;
}

MotionBase::MotionBase( OutputBase *target ):
  _target( target ),
  _output_base( target )
{
  if( _output_base->_input ) {
    _output_base->_input->disconnect();
  }
  _output_base->_input = this;
   app::console() << "MotionBase from OutputBase*" << endl;
}

MotionBase::~MotionBase()
{
  disconnect();
}

void MotionBase::step( float dt )
{
  _time += dt * _speed;
  update(); // update properties
  _previous_time = _time;
}

void MotionBase::disconnect()
{
  if( _output_base ) {
    _output_base->_input = nullptr;
    _output_base = nullptr;
    app::console() << "MotionBase disconnected" << endl;
  }
}

bool MotionBase::isFinished() const
{
  if( ! _continuous )
  {
    if( backward() ) {
      return time() <= 0.0f;
    }
    else {
      return time() >= getDuration();
    }
  }
  return false;
}

void MotionBase::resetTime()
{
  if( forward() )
  {
    _time = _previous_time = 0.0f;
  }
  else
  {
    _time = _previous_time = getDuration();
  }
}

//
//  Cue
//

void Cue::update()
{
  if( forward() && time() >= 0.0f && previousTime() < 0.0f )
    _cue();
  else if( forward() && time() <= 0.0f && previousTime() > 0.0f )
    _cue();
}


//
//  Motion
//
