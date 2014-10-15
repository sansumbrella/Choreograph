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
  float w = app::getWindowWidth();
  float h = app::getWindowHeight();
  mCurvePoints = {
    vec2( w * 0.08f, h * 0.86f ),
    vec2( w * 0.08f, h * 0.14f ),
    vec2( w * 0.92f, h * 0.14f ),
    vec2( w * 0.92f, h * 0.86f )
  };

  const float duration = 1.5f;

  // Ramp from anchor point to control point.
  auto ramp_a = makeRamp( mCurvePoints[0], mCurvePoints[1], duration );
  // Ramp from control point to anchor point.
  auto ramp_b = makeRamp( mCurvePoints[2], mCurvePoints[3], duration );

  // Lerp between control ramps.
  auto bezier_point = makeBlend<vec2>( ramp_a, ramp_b, 0.0f );

  timeline().setDefaultRemoveOnFinish( false );

  auto group = MotionGroup::create();

  group->add<vec2>( ramp_a, &mControlA );
  group->add<vec2>( ramp_b, &mControlB );
  group->add<float>( makeRamp( 0.0f, 1.0f, duration ), bezier_point->getMixOutput() );
  group->add<vec2>( bezier_point, &mCurvePoint )
    .startFn( [this] ( Motion<vec2> &m ) {
      mSegments.clear();
      mSegments.push_back( mCurvePoints[0] );
    } )
    .updateFn( [this] ( vec2 &pos ) {
      mSegments.push_back( pos );
    } );

  group->setFinishFn( [this] ( MotionGroup &group )
    {
      timeline().cue( [&group]
      {
        group.resetTime();
      }, 0.5f );
    } );

  timeline().add( std::move( group ) );

  // place things at initial timelined values.
  timeline().jumpTo( 0 );
}

void BezierConstruction::update( double dt )
{
  timeline().step( dt );
}

void BezierConstruction::draw()
{
  gl::ScopedColor color( Color::white() );
  gl::ScopedAlphaBlend blend( false );
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

  gl::drawString( "Bezier Path", mCurvePoint() - vec2( 0, 16 ) );
}
