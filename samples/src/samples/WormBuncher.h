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

#include "pockets/Scene.h"

struct WormSegment
{
#if defined(CINDER_MSW)
  WormSegment() = default;
  // We need to provide a move constructor for VS2013 to use instead of copy construction.
  // Clang auto-generates the move constructor for us, so it's not needed on OSX.
  // We should be able to declare move-ctor as default, but VS2013 doesn't support this yet.
  // http://msdn.microsoft.com/en-us/library/dn457344.aspx
  // WormSegment( WormSegment &&rhs ) = default;
  WormSegment( WormSegment &&rhs ):
    color( std::move( rhs.color ) ),
    alpha( std::move( rhs.alpha ) ),
    position( std::move( rhs.position ) ),
    orientation( std::move( rhs.orientation ) )
  {}
  WormSegment( const WormSegment &rhs ) = delete;
#endif

  ci::Color             color;
  ch::Output<float>     alpha = 0.0f;
  ch::Output<ci::vec3>  position;
  ch::Output<ci::quat>  orientation;
};

class WormBuncher : public pk::Scene
{
public:

  void setup() override;

  void update( ch::Time dt ) override;

  void connect( ci::app::WindowRef window ) override;

  void draw() override;

private:
  std::vector<WormSegment>  _segments;
};
