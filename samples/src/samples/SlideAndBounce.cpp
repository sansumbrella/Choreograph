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

#include "SlideAndBounce.h"

using namespace choreograph;
using namespace cinder;
using namespace std;

void SlideAndBounce::setup()
{

  // Create a procedural phrase that moves vertically on a sine wave.
  // Procedural phrases can evaluate any function you like.
  PhraseRef<vec2> bounce = makeProcedure<vec2>( 2.0, [] ( Time t, Time duration ) {
    return vec2( 0, sin( easeInOutQuad(t) * 6 * M_PI ) * 100.0f );
  } );

  // Create a ramp phrase from the left to the right side of the window.
  float w = (float)app::getWindowWidth();
  float x1 = w * 0.08f;
  float x2 = w - x1;
  PhraseRef<vec2> slide = makeRamp( vec2( x1, 0 ), vec2( x2, 0 ), 2.0f, EaseInOutCubic() );

  // Combine the slide and bounce phrases using an AccumulatePhrase.
  // By default, the accumulation operation sums all the phrase values with an initial value.
  float center_y = app::getWindowHeight() / 2.0f;
  PhraseRef<vec2> bounce_and_slide = makeAccumulator( vec2( 0, center_y ), bounce, slide );

  // Provide an explicit combine function.
  // In this case, we subtract each value from the initial value.
  PhraseRef<vec2> bounce_and_slide_negative = makeAccumulator( vec2( w, center_y ), bounce, slide, [] (const vec2 &a, const vec2 &b) {
    return a - b;
  } );

  // Apply our Sequences to Outputs.
  timeline().apply( &_position_a, bounce_and_slide );
  timeline().apply( &_position_b, bounce_and_slide_negative );
  timeline().apply( &_reference_bounce, bounce );
  timeline().apply( &_reference_slide, slide );

  // Place Outputs at initial sequence values.
  timeline().jumpTo( 0 );
}

void SlideAndBounce::update( Time dt )
{
  timeline().step( dt );
}

void SlideAndBounce::draw()
{
  gl::ScopedColor color( Color( CM_HSV, 0.72f, 1.0f, 1.0f ) );
  gl::drawSolidCircle( _position_a, 30.0f );

  gl::color( Color( CM_HSV, 0.96f, 1.0f, 1.0f ) );
  gl::drawSolidCircle( _position_b, 30.0f );

  // References are translated for visibility.
  float y = app::getWindowHeight() * 0.2f;
  gl::color( Color( CM_HSV, 0.15f, 1.0f, 1.0f ) );

  gl::drawStrokedCircle( _reference_bounce() + vec2( app::getWindowWidth() * 0.08f, y ), 4.0f );
  gl::drawStrokedCircle( _reference_slide() + vec2( 0, y ), 4.0f );
}
