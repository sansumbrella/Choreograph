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
  mCurvePoints = {
    vec2( 100, 600 ),
    vec2( 100, 100 ),
    vec2( 1180, 100 ),
    vec2( 1180, 600 )
  };

  const float duration = 1.5f;

  // Ramp from anchor point to control point.
  auto ramp_a = RampTo<vec2>::create( mCurvePoints[0], mCurvePoints[1], duration );
  // Ramp from control point to anchor point.
  auto ramp_b = RampTo<vec2>::create( mCurvePoints[2], mCurvePoints[3], duration );

  // Lerp between control ramps.
  auto bezier_point = MixPhrase<vec2>::create( ramp_a, ramp_b, 0.0f );

  timeline().setAutoRemove( false );

  timeline().apply( &mControlA, ramp_a );
  timeline().apply( &mControlB, ramp_b );
  timeline().apply( bezier_point->getMixOutput() ).then<RampTo>( 1.0f, duration );
  timeline().apply( &mCurvePoint, (PhraseRef<vec2>)bezier_point )
    .startFn( [this] ( Motion<vec2> &m ) {
      mSegments.clear();
    } )
    .updateFn( [this] ( vec2 &pos ) {
      mSegments.push_back( pos );
    } )
    .finishFn( [this] ( Motion<vec2> &m ) {
      timeline().cue( [this] {
        timeline().setTime( 0.0f );
      }, 0.5f ).continuous( false );
    } );

  Color first( 1.0f, 0.1f, 0.05f );
  Color curve( 1.0f, 0.0f, 1.0f );
}

void BezierConstruction::update( double dt )
{
  timeline().step( dt );
}

void BezierConstruction::draw()
{
  gl::ScopedColor color( Color::white() );
  Color curveColor( 1.0f, 1.0f, 0.0f );
  Color controlColor( 1.0f, 0.0f, 1.0f );
  Color lineColor( 0.0f, 1.0f, 1.0f );
  Color futureColor( 1.0f, 1.0f, 1.0f );

  // Draw our curve.
  gl::color( lineColor );
  gl::begin( GL_LINE_STRIP );
  for( auto &segment : mSegments ) {
    gl::vertex( segment );
  }
  gl::end();

  // Draw our curve's static control points.
  for( auto &point : mCurvePoints ) {
    gl::drawSolidCircle( point, 6.0f );
  }

  // Draw the paths traveled by our animating control points.
  gl::drawLine( mCurvePoints[0], mCurvePoints[1] );
  gl::drawLine( mCurvePoints[2], mCurvePoints[3] );

  // Draw our animating control points.
  gl::color( controlColor );
  gl::drawStrokedCircle( mControlA, 10.0f );
  gl::drawStrokedCircle( mControlB, 10.0f );

  // Draw our curve point's tangent line.
  gl::color( futureColor );
  gl::drawLine( mCurvePoint, mControlB );
  gl::color( curveColor );
  gl::drawLine( mControlA, mCurvePoint );
  // And our leading curve point.
  gl::drawStrokedCircle( mCurvePoint, 12.0f );
}
