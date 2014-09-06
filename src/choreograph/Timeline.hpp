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

#include "pockets/CollectionUtilities.hpp"

namespace choreograph
{

/*
 Holds a collection of Motions.
 Maybe variadic templates to specify an animation with different channel types,
 or one composed of n existing channels...
 */
class Timeline
{
public:

  //! Create a Sequence that is connected out to \a output.
  template<typename T>
  Motion<T>& move( T *output )
  { // remove any existing motions that affect the same variable (because that doesn't make sense within a single timeline)
    pk::vector_erase_if( &_motions, [=] (std::shared_ptr<MotionBase> m) { return m->_target == output; } );

    auto c = std::make_shared<Motion<T>>();
    c->sequence = std::make_shared<Sequence<T>>();
    c->output = output;
    c->_target = output;
    c->sequence->_initial_value = *output;
    _motions.push_back( c );
    return *c;
  }

  //! Create a Motion that plays \a sequence into \a output.
  template<typename T>
  Motion<T>& move( T *output, std::shared_ptr<Sequence<T>> sequence )
  { // remove any existing motions that affect the same variable (because that doesn't make sense within a single timeline)
    pk::vector_erase_if( &_motions, [=] (const std::shared_ptr<MotionBase> &m) { return m->_target == output; } );

    auto c = std::make_shared<Motion<T>>();
    c->sequence = sequence;
    c->output = output;
    c->_target = output;
    c->sequence->_initial_value = *output;
    _motions.push_back( c );
    return *c;
  }

  // Advance all current connections.
  void step( float dt )
  {
    for( auto &c : _motions )
    {
      c->step( dt );
    }

    if( _auto_clear )
    {
      pk::vector_erase_if( &_motions, [=] (const std::shared_ptr<MotionBase> &c ) { return !c->_continuous && c->time() >= c->getDuration(); } );
//      _motions.erase( std::remove_if( _motions.begin(), _motions.end(), [=] (const std::shared_ptr<MotionBase> &c ) { return !c->_continuous && c->time >= c->getDuration(); } ), _motions.end() );
    }
  }

  void remove( const std::shared_ptr<MotionBase> &motion )
  {
    _motions.erase( std::remove_if( _motions.begin(), _motions.end(), [=] (const std::shared_ptr<MotionBase> &c ) { return c == motion; } ), _motions.end() );
  }

private:
  bool                                      _auto_clear = true;
  std::vector<std::shared_ptr<MotionBase>>  _motions;
};

} // namespace choreograph
