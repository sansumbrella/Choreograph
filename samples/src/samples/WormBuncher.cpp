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

#include "WormBuncher.h"
#include "cinder/Rand.h"

using namespace choreograph;
using namespace cinder;

void WormBuncher::setup()
{
  const int count = 64;
  Sequence<quat> spin{ quat() };
  // would be easier as a floating point angle
  spin.then<RampTo>( glm::angleAxis<float>( M_PI / 2.0f, vec3( 0, 1, 0 ) ), 0.5f )
    .then<RampTo>( glm::angleAxis<float>( M_PI, vec3( 0, 1, 0 ) ), 0.5f )
    .then<RampTo>( glm::angleAxis<float>( M_PI * 3 / 2, vec3( 0, 1, 0 ) ), 0.5f )
    .then<RampTo>( glm::angleAxis<float>( 0, vec3( 0, 1, 0 ) ), 0.5f );

  for( int i = 0; i < count; ++i )
  {
    WormSegment segment;

    vec3 pos( 0 );
    pos.x = lmap<float>( i, 0, count - 1, 50, app::getWindowWidth() - 50 );
    pos.y = app::getWindowHeight() / 2 + sin( pos.x * 0.01f ) * 80.0f;

    segment.position().x = pos.x - 150.0f;
    segment.position().y = pos.y + 150.0f;

    segment.color = Color( CM_HSV, mix( 0.3f, 0.7f, randFloat() ), 1.0f, 0.95f );

    quat back = normalize( glm::angleAxis<float>( M_PI / 2.0f, vec3( 1, 0, 0 ) ) * glm::angleAxis<float>( M_PI / 4.0f, vec3( 0, 1, 0 ) ) );
    quat front;

    // Delay Motions by offsetting their start times.
    // Their values will be clamped to the start value until start time.
    float delay = (i * 0.05f);
    timeline().apply( &segment.position ).then<RampTo3>( pos, 0.44f, EaseOutCubic(), EaseOutQuad() ).setStartTime( delay );
    timeline().apply( &segment.orientation ).set( back ).then<RampTo>( front, 0.44f, EaseInOutAtan() ).setStartTime( delay );
    timeline().apply( &segment.alpha ).setStartTime( delay ).then<RampTo>( 1.0f, 0.16f );

    _segments.emplace_back( std::move( segment ) );
  }

}

void WormBuncher::connect( app::WindowRef window )
{
  auto seekPoint = [this] ( const ivec2 &point ) {
    vec3 center( point, 0.0f );
    vec3 bounds( 100 );
    float delay = 0.0f;
    for( auto &segment : _segments ) {
      vec3 pos = randVec3() * bounds + center;
      timeline().append( &segment.position ).hold( delay ).then<RampTo3>( pos, 0.5f, EaseInOutQuad(), EaseInOutCubic(), EaseInOutAtan() );

      delay += 0.005f;
    }
  };


#if defined( CINDER_COCOA_TOUCH )
  storeConnection( window->getSignalTouchesBegan().connect( [seekPoint] ( const app::TouchEvent &event ) {
    for( auto &touch : event.getTouches() ) {
      seekPoint( touch.getPos() );
    }
  } ) );
#else
  storeConnection( window->getSignalMouseDown().connect( [seekPoint] ( const app::MouseEvent &event ) {
    seekPoint( event.getPos() );
  } ) );
#endif
}

void WormBuncher::update( Time dt )
{
  timeline().step( dt );
}

void WormBuncher::draw()
{
  gl::ScopedBlendAlpha blend;

  gl::drawString( "Worm Buncher. Click/touch to interact.", vec2( 10, 30 ) );

  for( auto &segment : _segments )
  {
    gl::ScopedModelMatrix matrix;
    gl::translate( segment.position );
    gl::rotate( segment.orientation );

    gl::ScopedColor color( ColorA( segment.color, segment.alpha ) );
    gl::drawSolidCircle( vec2( 0 ), 20.0f );
  }
}
