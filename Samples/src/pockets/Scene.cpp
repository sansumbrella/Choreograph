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

#include "Scene.h"

using namespace pockets;
using namespace cinder;

Scene::Scene()
{ // start updating
  mUpdateConnection.store( app::App::get()->getSignalUpdate().connect( [this]()
  {
    update( _timer.getSeconds() * _animation_speed() );
    _timer.start();
  } ) );

  _timer.start();
}

Scene::~Scene()
{ // make sure nothing references us anymore
  mUpdateConnection.disconnect();
  disconnect();
  removeFromDisplay();
}

void Scene::baseDraw()
{
  gl::ScopedMatrices matrices;
  gl::setMatricesWindowPersp( app::getWindowSize() );
  gl::translate( _offset );

  draw();
}

void Scene::pause()
{
  _timer.stop();
  mUpdateConnection.block();
}

void Scene::resume()
{
  mUpdateConnection.resume();
  _timer.start();
}

void Scene::show( const app::WindowRef &window, bool useWindowBounds )
{
  mDisplayConnection.disconnect();
  mDisplayConnection.store( window->getSignalDraw().connect( 0, [this](){ baseDraw(); } ) );
  if( useWindowBounds ){ setBounds( window->getBounds() ); }
}
