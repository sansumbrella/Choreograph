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

#include "Timeline.h"
#include "detail/VectorManipulation.hpp"

using namespace choreograph;

void Timeline::step( Time dt )
{
  // Remove any motions that have stale pointers or that have completed playing.
  detail::erase_if( &_motions, [] ( const MotionBaseRef &motion ) { return (motion->getRemoveOnFinish() && motion->isFinished()) || (! motion->isValid()); } );

  // Update all animation outputs.
  for( auto &c : _motions ) {
    c->step( dt );
  }
}

void Timeline::jumpTo( Time time )
{
  // Remove any motions that have stale pointers or that have completed playing.
  detail::erase_if( &_motions, [] ( const MotionBaseRef &motion ) { return (motion->getRemoveOnFinish() && motion->isFinished()) || (! motion->isValid()); } );

  // Update all animation outputs.
  for( auto &c : _motions ) {
    c->jumpTo( time );
  }
}

void Timeline::setTime( Time time )
{
  // Remove any motions that have stale pointers or that have completed playing.
  detail::erase_if( &_motions, [] ( const MotionBaseRef &motion ) { return (motion->getRemoveOnFinish() && motion->isFinished()) || (! motion->isValid()); } );

  // Update all animation outputs.
  for( auto &c : _motions ) {
    c->setTime( time );
  }
}

void Timeline::remove( const MotionBaseRef &motion )
{
  detail::vector_remove( &_motions, motion );
}

void Timeline::remove( void *output )
{
  detail::erase_if( &_motions, [=] (const MotionBaseRef &m) { return m->getTarget() == output; } );
}

CueOptions Timeline::cue( const std::function<void ()> &fn, Time delay )
{
  auto cue = std::make_shared<Cue>( fn, delay );
  _motions.push_back( cue );

  return CueOptions( cue );
}
