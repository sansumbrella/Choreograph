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
  float w = (float)app::getWindowWidth();
  float h = (float)app::getWindowHeight();
  _curve_points = {
    vec2( w * 0.08f, h * 0.86f ),
    vec2( w * 0.08f, h * 0.14f ),
    vec2( w * 0.92f, h * 0.14f ),
    vec2( w * 0.92f, h * 0.86f )
  };

  const float duration = 1.5f;

  // Ramp from anchor point to control point.
  auto ramp_a = makeRamp( _curve_points[0], _curve_points[1], duration );
  // Ramp from control point to anchor point.
  auto ramp_b = makeRamp( _curve_points[2], _curve_points[3], duration );

  // Lerp between control ramps.
  auto bezier_point = makeBlend<vec2>( ramp_a, ramp_b, 0.0f );

  timeline().setDefaultRemoveOnFinish( false );

  auto group = std::make_shared<ch::Timeline>();
  group->setDefaultRemoveOnFinish( false );
  group->setRemoveOnFinish( false );

  // Animate our control points along their respective ramps.
  group->apply<vec2>( &_control_a, ramp_a );
  group->apply<vec2>( &_control_b, ramp_b );

  // Animate the mix of the bezier point from a to b.
  group->apply<float>( bezier_point->getMixOutput(), makeRamp( 0.0f, 1.0f, duration ) );

  // Apply the bezier_point animation to our curve point variable.
  group->apply<vec2>( &_curve_point, bezier_point )
    .startFn( [this] {
      _segments.clear();
      _segments.push_back( _curve_points[0] );
    } )
    .updateFn( [this] {
      _segments.push_back( _curve_point );
    } );

  // When all our animations finish, cue the group to restart after a delay.
  group->setFinishFn( [this, group] () {
    timeline()
      .cue( [group] {
        group->resetTime();
      }, 0.5f )
      .removeOnFinish( true );
  } );

  // Move our grouping timeline onto our main timeline.
  // This will update our group as the main timeline progresses.
  timeline().addShared( group );

  // place things at initial timelined values.
  timeline().jumpTo( 0 );
}

void BezierConstruction::update( Time dt )
{
  timeline().step( dt );
}

void BezierConstruction::draw()
{
  gl::ScopedColor       color( Color::white() );
  gl::ScopedBlendAlpha  blend;
  Color curve_color( 1.0f, 1.0f, 0.0f );
  Color control_color( 1.0f, 0.0f, 1.0f );
  Color line_color( 0.0f, 1.0f, 1.0f );
  Color future_color( 1.0f, 1.0f, 1.0f );

  // Draw our curve.
  gl::color( line_color );
  gl::begin( GL_LINE_STRIP );
  for( auto &segment : _segments ) {
    gl::vertex( segment );
  }
  gl::end();

  // Draw our curve's static control points.
  for( auto &point : _curve_points ) {
    gl::drawSolidCircle( point, 6.0f );
  }

  // Draw the paths traveled by our animating control points.
  gl::drawLine( _curve_points[0], _curve_points[1] );
  gl::drawLine( _curve_points[2], _curve_points[3] );

  // Draw our animating control points.
  gl::color( control_color );
  gl::drawStrokedCircle( _control_a, 10.0f );
  gl::drawStrokedCircle( _control_b, 10.0f );

  // Draw our curve point's tangent line.
  gl::color( future_color );
  gl::drawLine( _curve_point, _control_b );
  gl::color( curve_color );
  gl::drawLine( _control_a, _curve_point );
  // And our leading curve point.
  gl::drawStrokedCircle( _curve_point, 12.0f );

  gl::drawString( "Bezier Path", _curve_point() - vec2( 0, 16 ) );
}
