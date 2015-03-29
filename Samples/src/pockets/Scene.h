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

#pragma once
#include "Pockets.h"
#include "ConnectionManager.h"
#include "cinder/app/Window.h"
#include "cinder/Timer.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "choreograph/Choreograph.h"

namespace pockets
{
  /**
   A basic application layer encapsulating input and view controls

   Renderable
   Updatable, pausable (always calls its own update on app signal unless paused)
   Listens for UI events

   Eventually, would like to make a simple scene-graph-like setup of:
   Connectable -> can connect itself to window stuff
   Renderable -> can be drawn to screen
   Maybe Runnable -> can/expects to be updated at 60Hz

   Scene graph would be useful primarily for laying out a simple GUI.
   Simple scene graph is implemented as Nodes in scene2d.
   */
  typedef std::shared_ptr<class Scene> SceneRef;
	class Scene
	{
	public:
    typedef std::function<void ()> Callback;
    Scene();
    virtual ~Scene();
    /// Set up scene when OpenGL context is guaranteed to exist (in or after app::setup())
    virtual void  setup() {}

    /// connect to receive user interaction events (of subclasses' choosing)
    /// Consider storing reference to the window for passing into potential subviews
    virtual void  connect( ci::app::WindowRef window ){}

    /// stop receiving window UI events
    void          disconnect(){ mUIConnections.disconnect(); }

    /// temporarily freeze updates
    void          pause();

    /// continue receiving updates
    void          resume();

    /// update content
    virtual void  update( ch::Time dt ){}

    void baseDraw();

    /// render content
    virtual void  draw() {}

    /// Returns a pointer to the Scene's offset for animation.
    ch::Output<ci::vec2>* getOffsetOutput() { return &_offset; }

    void setOffset( const ci::vec2 &offset ) { _offset = offset; }

    ch::Output<ch::Time>* getAnimationSpeedOutput() { return &_animation_speed; }

    /// returns the bounds of the controller in points
    ci::Area      getBounds() const { return _bounds; }

    /// set the region of screen into which we should draw this view
    void          setBounds( const ci::Area &points ){ _bounds = points; }

    /// manage the lifetime of the given connection and control with block/unblock
    void          storeConnection( const ci::signals::Connection &c ) { mUIConnections.store( c ); }

    void show( const ci::app::WindowRef &window, bool useWindowBounds = true );
    /// Returns a reference to our timeline.
    choreograph::Timeline& timeline() { return _timeline; }

	private:
    ch::Output<ci::vec2>    _offset = ci::vec2( 0 );
    ch::Output<ch::Time>    _animation_speed = 1;
    ci::Area                _bounds;
    ci::Timer               _timer;

    choreograph::Timeline   _timeline;
    /// UI connections
    ConnectionManager       mUIConnections;
    /// window update/display connections
    ConnectionManager       mDisplayConnection;
    ConnectionManager       mUpdateConnection;

    void      removeFromDisplay(){ mDisplayConnection.disconnect(); }
	};
}
