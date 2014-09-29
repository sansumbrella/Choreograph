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

#include "Hello.h"
#include "cinder/Rand.h"

using namespace choreograph;
using namespace cinder;

void Hello::setup()
{
  const int count = 64;
  Sequence<quat> spin{ quat() };
  // would be easier as a floating point angle
  spin.then<RampTo>( glm::angleAxis<float>( M_PI / 2, vec3( 0, 1, 0 ) ), 0.5f )
    .then<RampTo>( glm::angleAxis<float>( M_PI, vec3( 0, 1, 0 ) ), 0.5f )
    .then<RampTo>( glm::angleAxis<float>( M_PI * 3 / 2, vec3( 0, 1, 0 ) ), 0.5f )
    .then<RampTo>( glm::angleAxis<float>( 0, vec3( 0, 1, 0 ) ), 0.5f );

  for( int i = 0; i < count; ++i )
  {
    Thing thing;

    vec3 pos( 0 );
    pos.x = lmap<float>( i, 0, count - 1, 50, app::getWindowWidth() - 50 );
    pos.y = app::getWindowHeight() / 2 + sin( pos.x * 0.01f ) * 80.0f;

    thing.position().x = pos.x - 150.0f;
    thing.position().y = pos.y + 150.0f;

    thing.color = Color( CM_HSV, mix( 0.3f, 0.7f, randFloat() ), 1.0f, 0.95f );

    quat back = normalize( glm::angleAxis<float>( M_PI / 2.0f, vec3( 1, 0, 0 ) ) * glm::angleAxis<float>( M_PI / 4.0f, vec3( 0, 1, 0 ) ) );
    quat front;

    // We could delay a subset of motions by prepending a Hold.
    // Maybe add a method to timeline that is like append, but actually builds a Cue to apply the Sequence.
    float delay = (i * 0.05f);
    timeline().apply( &thing.position ).then<RampTo3>( pos, 0.44f, EaseOutCubic(), EaseOutQuad() ).delay( delay );
    timeline().apply( &thing.orientation ).set( back ).then<RampTo>( front, 0.44f, EaseInOutAtan() ).delay( delay );
    timeline().apply( &thing.alpha ).delay( delay ).then<RampTo>( 1.0f, 0.16f );

    mThings.push_back( thing );
  }
}

void Hello::connect( app::WindowRef window )
{
  storeConnection( window->getSignalMouseDown().connect( [this] ( const app::MouseEvent &event ) {
    vec3 center( event.getPos(), 0.0f );
    vec3 bounds( 100 );
    float delay = 0.0f;
    for( auto &thing : mThings ) {
      vec3 pos = randVec3f() * bounds + center;
      timeline().append( &thing.position ).hold( delay ).then<RampTo3>( pos, 0.5f, EaseInOutQuad(), EaseInOutCubic(), EaseInOutAtan() );

      delay += 0.005f;
    }
  } ) );
}

void Hello::update( double dt )
{
  timeline().step( dt );
}

void Hello::draw()
{
  gl::ScopedMatrices matrices;
  gl::setMatricesWindowPersp( app::getWindowSize(), 60.0f, 1.0f, 2000.0f );

  gl::enableAlphaBlending();

  for( auto &thing : mThings )
  {
    gl::ScopedModelMatrix matrix;
    gl::translate( thing.position );
    gl::rotate( thing.orientation );

    gl::color( ColorA( thing.color, thing.alpha ) );
    gl::drawSolidCircle( vec2( 0 ), 20.0f );
  }
}
