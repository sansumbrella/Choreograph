//
//  Scene.cpp
//  WordShift
//
//  Created by David Wicks on 2/21/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "Scene.h"
#include "cinder/app/App.h"

using namespace pockets;
using namespace cinder;

Scene::Scene()
{ // start updating
  mUpdateConnection.store( app::App::get()->getSignalUpdate().connect( [this]()
  {
    update( mTimer.getSeconds() );
    mTimer.start();
  } ) );
}

Scene::~Scene()
{ // make sure nothing references us anymore
  mUpdateConnection.disconnect();
  disconnect();
  removeFromDisplay();
}

void Scene::block()
{
  mUIConnections.block();
}

void Scene::unblock()
{
  mUIConnections.resume();
}

void Scene::pause()
{
  mTimer.stop();
  mUpdateConnection.block();
  customPause();
}

void Scene::resume()
{
  mUpdateConnection.resume();
  customResume();
}

Scene::Callback Scene::vanishCompleteFn( Scene::Callback finishFn)
{
  return [this, finishFn](){ removeFromDisplay(); if( finishFn ){ finishFn(); } };
}

void Scene::show( app::WindowRef window, bool useWindowBounds )
{
  mDisplayConnection.disconnect();
  mDisplayConnection.store( window->getSignalDraw().connect( 1, [this](){ draw(); } ) );
  if( useWindowBounds ){ setBounds( window->getBounds() ); }
  appear();
}
