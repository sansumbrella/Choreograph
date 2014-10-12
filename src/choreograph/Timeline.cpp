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

void Timeline::removeFinishedAndInvalidMotions()
{
  detail::erase_if( &_motions, [] ( const TimelineItemUniqueRef &motion ) { return (motion->getRemoveOnFinish() && motion->isFinished()) || motion->isInvalid(); } );
}

void Timeline::removeInvalidMotions()
{
  detail::erase_if( &_motions, [] ( const TimelineItemUniqueRef &motion ) { return motion->isInvalid(); } );
}

void Timeline::removeFinishedMotions()
{
  detail::erase_if( &_motions, [] ( const TimelineItemUniqueRef &motion ) { return (motion->getRemoveOnFinish() && motion->isFinished()); } );
}

void Timeline::step( Time dt )
{
  // It is ~10% faster to only iterate for removal once.
  // This means our finished motions won't be removed until the beginning of the next frame.
  removeFinishedAndInvalidMotions();

//  removeInvalidMotions();

  _updating = true;
  // Update all animation outputs.
  for( auto &c : _motions ) {
    c->step( dt );
  }
  _updating = false;

//  removeFinishedMotions();

  // Copy any queued motions
  std::copy( std::make_move_iterator( _queue.begin() ), std::make_move_iterator( _queue.end() ), std::back_inserter( _motions ) );
  _queue.clear();

}

void Timeline::jumpTo( Time time )
{
  removeFinishedAndInvalidMotions();

  // Update all animation outputs.
  for( auto &c : _motions ) {
    c->jumpTo( time );
  }
}

void Timeline::setTime( Time time )
{
  removeFinishedAndInvalidMotions();

  // Update all animation outputs.
  for( auto &c : _motions ) {
    c->setTime( time );
  }
}

void Timeline::remove( void *output )
{
  detail::erase_if( &_motions, [=] (const TimelineItemUniqueRef &m) { return m->getTarget() == output; } );
}

void Timeline::add( TimelineItemUniqueRef motion )
{
  _motions.emplace_back( std::move( motion ) );
}

CueOptions Timeline::cue( const std::function<void ()> &fn, Time delay )
{
  auto cue = std::unique_ptr<Cue>( new Cue( fn, delay ) );
  CueOptions options( *cue );

  if( _updating )
    _queue.emplace_back( std::move( cue ) );
  else
    _motions.emplace_back( std::move( cue ) );

  return options;
}
