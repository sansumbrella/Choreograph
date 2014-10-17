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

#include "Sink.h"

#include "cinder/Rand.h"

using namespace std;
using namespace cinder;
using namespace choreograph;

void Sink::setup()
{
  auto redMotion = timeline().apply( &mBallY );
  redMotion.startFn( []( Motion<float> &m ) { app::console() << "Start Red Ball" << endl; } );
  redMotion.set( 5.0f ).then<RampTo>( 500.0f, 1.0f, EaseInOutQuad() )
            .then<Hold>( 500.0f, 2.0f )
            .then<RampTo>( 700.0f, 1.0f, EaseNone() )
            .then<RampTo>( 200.0f, 1.0f );

  auto blueMotion = timeline().apply( &mBall2 );
  blueMotion.startFn( [] ( Motion<vec2> &m ) { app::console() << "Start Blue Ball" << endl; } )
            .finishFn( [] ( Motion<vec2> &m ) { m.setPlaybackSpeed( m.getPlaybackSpeed() * -1.0f ); } )
            .updateFn( [this] (vec2 &v) {
              vec2 size = app::getWindowSize();
              float shortest = std::min( v.x, size.x - v.x );
              shortest = std::min( shortest, size.y - v.y );
              shortest = std::min( shortest, v.y );
              mBallRadius = 12.0f + shortest / 2;

              v.x *= 0.5f; // change the output interpolated value.
              // v *= 0.5f;
            } );

  blueMotion.then<RampTo>( vec2( app::getWindowSize() ) / 2.0f, 1.0f )
            .then<RampTo>( vec2( app::getWindowSize() ), 0.66f )
            .then<RampTo2>( vec2( 0, 0 ), 2.0f, EaseNone(), EaseInOutCubic() );

}

void Sink::connect( ci::app::WindowRef window )
{
  storeConnection( window->getSignalMouseDown().connect( [this] ( app::MouseEvent &event ) {
    timeline().apply( &mMouseApply ).hold( 0.1f ).then<RampTo>( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );
    timeline().append( &mMouseAppend ).hold( 0.1f ).then<RampTo>( vec2( event.getPos() ), 1.0f, EaseInOutCubic() );

    timeline().append( &mOrientation ).then<RampTo>( glm::angleAxis( (float)(randFloat() * M_PI * 2), randVec3f() ), 1.0f, EaseInOutCubic() );

    quat step = glm::angleAxis<float>( M_PI / 2, vec3( 0, 1, 0 ) );
    quat target = mCircularOrientation() * step;
    timeline().apply( &mCircularOrientation ).then<RampTo>( normalize( target ), 0.33f, EaseOutQuad() );
  } ) );

  storeConnection( window->getSignalKeyDown().connect( [this] ( app::KeyEvent &event ) {
    timeline().apply( &mArced )
      .then<RampTo2>( vec2( app::getWindowSize() ), 3.0f, EaseNone(), EaseInOutQuint() )
      .then<RampTo2>( vec2( 0, app::getWindowHeight() / 2.0f ), 2.0f, EaseNone(), EaseInOutAtan() );
  } ) );
}

void Sink::update( Time dt )
{
  timeline().step( dt );
}

void Sink::draw()
{
  gl::ScopedMatrices matrices;

  gl::ScopedAlphaBlend blend( false );
  gl::drawString( "Dev Junk. Click/touch to interact.", vec2( 10, 30 ) );

  gl::disableDepthRead();
  gl::disableDepthWrite();

  gl::ScopedColor red( Color( 1.0f, 0.0f, 0.0f ) );
  gl::drawSolidCircle( vec2( 200.0f, mBallY() ), 120.0f );
  gl::ScopedColor blue( Color( 0.0f, 0.0f, 1.0f ) );
  gl::drawSolidCircle( mBall2, mBallRadius );

  gl::ScopedColor green( Color( 0.0f, 1.0f, 0.0f ) );
  gl::drawSolidCircle( mMouseAppend, 40.0f );

  gl::ScopedColor steelblue( Color( "steelblue" ) );
  gl::drawSolidCircle( mMouseApply, 30.0f );

  gl::ScopedColor magenta( Color( "magenta" ) );
  gl::drawSolidCircle( mArced, 30.0f );

  gl::enableDepthRead();
  gl::enableDepthWrite();
  {
    gl::ScopedMatrices orientMatrices;
    gl::translate( app::getWindowCenter() );
    gl::rotate( mOrientation );
    const int n = 3;
    for( int i = 0; i < n; ++i ) {
      vec3 pos = glm::mix( vec3( -100.0f, 0.0f, 0.0f ), vec3( 100.0f, 0.0f, 0.0f ), i / (n - 1.0f) );
      float size = mix( 20.0f, 60.0f, i / (n - 1.0f) );
      gl::drawColorCube( pos, vec3( size ) );
    }
  }
  {
    gl::ScopedMatrices orientMatrices;
    gl::translate( vec2( app::getWindowWidth() * 0.75f, app::getWindowHeight() * 0.25f ) );
    gl::rotate( mCircularOrientation );
    gl::drawColorCube( vec3( 0 ), vec3( 50.0f ) );
  }

  gl::disableDepthRead();
  gl::disableDepthWrite();
}
