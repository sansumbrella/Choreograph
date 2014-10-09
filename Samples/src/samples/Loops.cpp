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

#include "Loops.h"

using namespace choreograph;
using namespace cinder;

void Loops::setup()
{
  float left = 100.0f;
  float right = app::getWindowWidth() - left;
  PhraseRef<vec2> leftToRight = makeRamp( vec2( left, 0.0f ), vec2( right, 0.0f ), 1.0f, EaseInOutQuad() );

  //=====================================================
  // Looping Motions. Use for things that loop "forever."
  //=====================================================

  Output<vec2>  loopTarget;
  timeline().apply( &loopTarget, leftToRight )
    .finishFn( [] ( Motion<vec2> &m ) {
      m.resetTime();
    } );  // Motion should play forever.


  Output<vec2>  pingPongTarget;
  timeline().apply( &pingPongTarget, leftToRight )
    .finishFn( [] ( Motion<vec2> &m ) {
      // reverse Motion direction on finish.
      m.setPlaybackSpeed( m.getPlaybackSpeed() * -1 );
      // Start each cycle from "zero" to keep in sync with loopTarget timing.
      m.resetTime();
    } );

  Output<vec2>  pingPongSlowerTarget;
  timeline().apply( &pingPongSlowerTarget, leftToRight )
    .finishFn( [] ( Motion<vec2> &m ) {
      // Reverse and slow Motion on finish.
      m.setPlaybackSpeed( m.getPlaybackSpeed() * -0.9f );
      // If we're unbearably slow, stop looping.
      if( std::abs( m.getPlaybackSpeed() ) < 0.2f ) {
        m.finishFn( [] ( Motion<vec2> &m ) {} );
      }
      else {
        m.resetTime();
      }
    } );

  //==========================================================
  // Looping Motion Group. Use for separate motions that loop
  // with each other, but have different durations.
  //==========================================================

  SequenceRef<vec2> positionSequence = createSequence( vec2( app::getWindowSize() ) * vec2( 0.66, 1 ) + vec2( 0, 50 ) );
  SequenceRef<vec3> rotationSequence = createSequence( vec3( M_PI / 2, 0, 0 ) );

  rotationSequence->then<RampTo>( vec3( 4 * M_PI, 2 * M_PI, 0 ), 1.0f, EaseOutQuint() );
  positionSequence->then<RampTo>( vec2( app::getWindowSize() ) * vec2( 0.66, 0.5 ), 0.5f, EaseOutAtan() );

  auto group = std::make_shared<MotionGroup>();
  group->add( positionSequence, &_position );
  group->add( rotationSequence, &_rotation );
  // start grouped motions after a 0.5 second hold on their start values.
  group->setStartTime( 0.5f );
  group->setFinishFn( [] ( MotionGroup &group ) {
    group.setPlaybackSpeed( group.getPlaybackSpeed() * -1 );
    group.resetTime();
  } );

  timeline().add( group );

  //=====================================================
  // Looping Phrases. Use for a finite number of loops.
  //=====================================================

  Output<vec2> loopPhraseTarget;
  timeline().apply( &loopPhraseTarget ).then( makeRepeat( leftToRight, 7.5f ) );

  Output<vec2> pingPongPhraseTarget;
  timeline().apply( &pingPongPhraseTarget ).then( makePingPong( leftToRight, 7.5f ) );

  mTargets.push_back( { loopTarget, Color( 1, 0, 1 ) } );
  mTargets.push_back( { pingPongTarget, Color( 1, 0, 1 ) } );
  mTargets.push_back( { pingPongSlowerTarget, Color( 0, 1, 1 ) } );
  mTargets.push_back( { loopPhraseTarget, Color( 1, 1, 0 ) } );
  mTargets.push_back( { pingPongPhraseTarget, Color( 1, 1, 0 ) } );
}

void Loops::update( double dt )
{
  timeline().step( dt );
}

void Loops::draw()
{
  gl::ScopedModelMatrix matrix;
  gl::setMatricesWindowPersp( app::getWindowSize() );

  {
    gl::ScopedModelMatrix singleDotMatrix;

    const float y_step = 100.0f;
    gl::translate( vec2( 0, (app::getWindowHeight() - y_step * mTargets.size()) / 2 ) );

    for( auto &target : mTargets ) {
      gl::ScopedColor color( target._color );
      gl::drawSolidCircle( target._position, 24.0f );
      gl::translate( vec2( 0, y_step ) );
    }
  }

  gl::ScopedColor color( Color( CM_HSV, 0.575f, 1.0f, 1.0f ) );
  gl::translate( _position );
  gl::multModelMatrix( glm::eulerAngleYXZ( _rotation().y, _rotation().x, _rotation().z ) );
  gl::drawSolidCircle( vec2( 0 ), 36.0f );
}
