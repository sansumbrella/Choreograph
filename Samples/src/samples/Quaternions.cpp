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

#include "Quaternions.h"

#include "cinder/Rand.h"

using namespace std;
using namespace cinder;
using namespace choreograph;

void Quaternions::setup()
{
  timeline().apply( &_continuous_orientation )
    .then<RampTo>( glm::angleAxis( (float)(randFloat() * M_PI * 2), randVec3f() ), 1.0f, EaseInOutCubic() )
    .finishFn( [this] (Motion<quat> &m) {
      m.sliceSequence( 0, 0 );
      m.resetTime();
      m.getSequence().then<RampTo>( glm::angleAxis( (float)(randFloat() * M_PI * 2), randVec3f() ), 1.0f, EaseInOutCubic() );
    } );
}

void Quaternions::rotateMore( ch::MotionOptions<ci::quat> &options )
{
  auto end = normalize( options.getSequence().getEndValue() );

  auto axis = vec3( 1, 0, 0 );
  float coin = randFloat( 1.0f );
  if( coin < 0.33f ) {
    axis = vec3( 0, 1, 0 );
  }
  else if( coin < 0.66f ) {
    axis = vec3( 0, 0, 1 );
  }

  quat step = glm::angleAxis<float>( M_PI / 2, axis );
  quat target = end * step;

  options.then<RampTo>( normalize( target ), 0.5f, EaseOutQuad() );
}

void Quaternions::connect( ci::app::WindowRef window )
{
  storeConnection( window->getSignalTouchesBegan().connect( [this] ( app::TouchEvent &event ) {
    auto apply_options = timeline().apply( &_apply_orientation );
    rotateMore( apply_options );

    auto append_options = timeline().append( &_append_orientation );
    rotateMore( append_options );
  } ) );

}

void Quaternions::update( Time dt )
{
  timeline().step( dt );
}

void Quaternions::draw()
{
  gl::ScopedMatrices matrices;

  gl::enableDepthRead();
  gl::enableDepthWrite();
  {
    gl::ScopedMatrices orientMatrices;
    gl::translate( app::getWindowCenter() );
    gl::rotate( _continuous_orientation );
    const int n = 3;
    for( int i = 0; i < n; ++i ) {
      vec3 pos = glm::mix( vec3( -100.0f, 0.0f, 0.0f ), vec3( 100.0f, 0.0f, 0.0f ), i / (n - 1.0f) );
      float size = mix( 20.0f, 60.0f, i / (n - 1.0f) );
      gl::drawColorCube( pos, vec3( size ) );
    }
  }
  {
    gl::ScopedMatrices orientMatrices;
    gl::translate( vec2( app::getWindowWidth() * 0.75f, app::getWindowHeight() * 0.33f ) );
    gl::rotate( _append_orientation );
    gl::drawColorCube( vec3( 0 ), vec3( 50.0f ) );
  }
  {
    gl::ScopedMatrices orientMatrices;
    gl::translate( vec2( app::getWindowWidth() * 0.25f, app::getWindowHeight() * 0.33f ) );
    gl::rotate( _apply_orientation );
    gl::drawColorCube( vec3( 0 ), vec3( 50.0f ) );
  }

  gl::disableDepthRead();
  gl::disableDepthWrite();
}
