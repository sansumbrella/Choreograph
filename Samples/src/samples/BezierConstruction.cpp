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

#include "BezierConstruction.h"

using namespace choreograph;
using namespace cinder;

void BezierConstruction::setup()
{
  Output<vec2>  a;
  Output<vec2>  b;
  Output<vec2>  curvePoint;

  const float duration = 2.0f;

  // Ramp from anchor point to control point.
  auto ramp_a = RampTo<vec2>::create( vec2( 100, 100 ), vec2( 100, 600 ), duration );
  // Ramp from control point to anchor point.
  auto ramp_b = RampTo<vec2>::create( vec2( 1180, 600 ), vec2( 1180, 100 ), duration );

  // Lerp between control ramps.
  auto bezier_point = MixPhrase<vec2>::create( ramp_a, ramp_b, 0.0f );

  timeline().apply( &a, ramp_a );
  timeline().apply( &b, ramp_b );
  timeline().apply( bezier_point->getMixOutput() ).then<RampTo>( 1.0f, duration );
  timeline().apply( &curvePoint, (PhraseRef<vec2>)bezier_point )
    .updateFn( [this] ( vec2 &pos ) {
      mSegments.push_back( pos );
    } );

  Color first( 1.0f, 0.1f, 0.05f );
  Color curve( 1.0f, 0.0f, 1.0f );
  mTargets.push_back( { a, first } );
  mTargets.push_back( { b, first } );
  mTargets.push_back( { curvePoint, curve } );
}

void BezierConstruction::update( double dt )
{
  timeline().step( dt );
}

void BezierConstruction::draw()
{
  gl::begin( GL_LINE_STRIP );
  for( auto &segment : mSegments ) {
    gl::vertex( segment );
  }
  gl::end();

  for( auto &target : mTargets ) {
    gl::ScopedColor color( target._color );
    gl::drawSolidCircle( target._position, 10.0f );
  }
}
