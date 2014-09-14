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
#include "Motion.h"

namespace choreograph
{
  //! Remove all elements from \a container that match \a compare
  template<class CONTAINER_TYPE, class COMPARATOR>
  void erase_if( CONTAINER_TYPE *container, COMPARATOR compare )
  {
    container->erase( std::remove_if( container->begin(),
                                     container->end(),
                                     compare ),
                     container->end() );
  }

  //! Remove all copies of \a element from \a vec
  template<class ELEMENT_TYPE>
  void vector_remove( std::vector<ELEMENT_TYPE> *vec, const ELEMENT_TYPE &element )
  {
    vec->erase( std::remove_if( vec->begin()
                               , vec->end()
                               , [element](const ELEMENT_TYPE &e){ return e == element; } )
               , vec->end() );
  }

/*
 Holds a collection of Motions and updates them through time.
 */
class Timeline
{
public:

  //
  // Sequence creation. Safe methods.
  //

  template<typename T>
  Motion<T>& move( Output<T> *output )
  {
    return move( output, std::make_shared<Sequence<T>>( *output ) );
  }

  template<typename T>
  Motion<T>& move( Output<T> *output, const SequenceRef<T> &sequence )
  { // We don't need to remove here, since previous parents will be invalidated through the Output/Motion relationship.
    auto motion = std::make_shared<Motion<T>>( output, sequence );
    _motions.push_back( motion );
    return *motion;
  }

  //
  // Sequence creation. Bare pointer methods.
  //

  //! Create a Sequence that is connected out to \a output.
  template<typename T>
  Motion<T>& move( T *output )
  {
    return move( output, std::make_shared<Sequence<T>>( *output ) );
  }

  //! Create a Motion that plays \a sequence into \a output.
  template<typename T>
  Motion<T>& move( T *output, const SequenceRef<T> &sequence )
  { // Remove any existing motions that affect the same variable.
    // This is a raw pointer, so we don't know about any prior relationships.
    remove( output );

    auto motion = std::make_shared<Motion<T>>( output, sequence );
    _motions.push_back( motion );
    return *motion;
  }

  //
  // Time manipulation.
  //

  //! Advance all current motions.
  void step( float dt );

  //
  // Sequence manipulation.
  //

  //! Add phrases to the end of the Sequence currently connected to \a output.
  template<typename T>
  Sequence<T>& queue( T *output )
  {
    for( auto &motion : _motions ) {
      if( motion->getTarget() == output ) {
        return std::static_pointer_cast<Motion<T>>( motion )->getSequence();
      }
    }
    return move( output ).getSequence();
  }

  //! Add phrases to the end of the Sequence currently connected to \a output.
  template<typename T>
  Sequence<T>& queue( Output<T> *output )
  {
    for( auto &motion : _motions ) {
      if( motion->getTarget() == output ) {
        return std::static_pointer_cast<Motion<T>>( motion )->getSequence();
      }
    }
    return move( output ).getSequence();
  }

  //! Remove specific motion.
  void remove( const std::shared_ptr<MotionBase> &motion );

  //! Remove motion associated with specific output.
  template<typename T>
  void remove( T *output )
  {
    erase_if( &_motions, [=] (std::shared_ptr<MotionBase> m) { return m->getTarget() == output; } );
  }

  //! Removes all motions from this timeline.
  void clear() { _motions.clear(); }

  //! Returns true if there are no motions on this timeline.
  bool empty() const { return _motions.empty(); }

  //! Returns the number of motions on this timeline.
  size_t size() const { return _motions.size(); }
private:
  bool                                      _auto_clear = true;
  std::vector<std::shared_ptr<MotionBase>>  _motions;
};

} // namespace choreograph
