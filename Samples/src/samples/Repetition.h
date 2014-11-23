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

///
/// Demonstrates different kinds of looping effects:
/// Motion looping, Group looping, and Loop Phrases.
///
class Repetition : public pk::Scene
{
public:
  void setup() override;
  void update( ch::Time dt ) override;
  void draw() override;
private:
  struct Point {
#if defined(CINDER_MSW)
    Point( ch::Output<ci::vec2> &&position, const ci::Color &color, const std::string &description ):
      _position( std::move( position ) ),
      _color( color ),
      _description( description )
    {}
    Point() = default;
    Point( Point &&rhs ):
      _position( std::move( rhs._position ) ),
      _color( std::move( rhs._color ) ),
      _description( std::move( rhs._description) )
    {}
    Point( const Point &rhs ) = delete;
#endif
    ch::Output<ci::vec2>  _position;
    ci::Color             _color;
    std::string           _description;
  };

  std::vector<Point> mTargets;

  // For grouped motion, we'll use these two properties.
  ch::Output<ci::vec2>  _position;
  ch::Output<ci::vec3>  _rotation;
};
