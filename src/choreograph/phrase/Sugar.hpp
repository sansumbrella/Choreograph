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

///
/// \file
/// Free functions to make creating certain meta-phrases easier.
///
/// These are in a separate file, not included by default, since I'm not
/// sold on their utility.
///
/// Note that the compiler can't deduce the type of T for PhraseRef<T> from DerivedRef<T>
/// so you may need to help it out. e.g. `loopPhrase<vec3>( DerivedRef<vec3>, 2 );`
///

namespace choreograph
{

/// Create a LoopPhrase that loops \a source Phrase \a numLoops times.
/// \see LoopPhrase<T>::create()
template<typename T>
PhraseRef<T> loopPhrase( const PhraseRef<T> &source, float numLoops, Time inflectionPoint = 0.0f )
{
  return LoopPhrase<T>::create( source, numLoops, inflectionPoint );
}

template<typename T>
PhraseRef<T> pingPongPhrase( const PhraseRef<T> &source, float numLoops, Time inflectionPoint = 0.0f )
{
  return PingPongPhrase<T>::create( source, numLoops, inflectionPoint );
}

} // namespace choreograph
