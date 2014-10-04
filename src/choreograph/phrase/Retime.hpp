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

#include "choreograph/Phrase.hpp"

namespace choreograph
{

template<typename T>
class LoopPhrase : public Phrase<T>
{
public:
  /// Create a Phrase that loops \a source \a numLoops times.
  LoopPhrase( const PhraseRef<T> &source, float numLoops, Time inflectionPoint = 0.0f ):
    Phrase<T>( source->getDuration() * numLoops ),
    _source( source ),
    _inflection_point( inflectionPoint )
  {}

  static std::shared_ptr<LoopPhrase<T>> create( const PhraseRef<T> &source, float numLoops, Time inflectionPoint = 0.0f )
  {
    return std::make_shared<LoopPhrase<T>>( source, numLoops, inflectionPoint );
  }

  T getValue( Time atTime ) const override { return _source->getValueWrapped( atTime, _inflection_point ); }
  T getStartValue() const override { return _source->getStartValue(); }
  T getEndValue() const override { return _source->getEndValue(); }
private:
  PhraseRef<T>  _source;
  Time          _inflection_point;
};

/// Free function to make creating looped phrases easier in client code.
template<typename T>
PhraseRef<T> loopPhrase( const PhraseRef<T> &source, float numLoops, Time inflectionPoint = 0.0f )
{
  return LoopPhrase<T>::create( source, numLoops, inflectionPoint );
}

} // namespace choreograph
