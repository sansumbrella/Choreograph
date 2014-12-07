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

#include "RootNode.h"

using namespace cinder;
using namespace pockets;
using namespace cobweb;

void RootNode::connect( ci::app::WindowRef window )
{
  storeConnection( window->getSignalTouchesBegan().connect( [this]( app::TouchEvent &event )
                                                           {
                                                             if( deepTouchesBegan( event ) )
                                                             { event.setHandled(); }
                                                           } ) );
  storeConnection( window->getSignalTouchesMoved().connect( [this]( app::TouchEvent &event )
                                                           {
                                                             if( deepTouchesMoved( event ) )
                                                             { event.setHandled(); }
                                                           } ) );
  storeConnection( window->getSignalTouchesEnded().connect( [this]( app::TouchEvent &event )
                                                           {
                                                             if( deepTouchesEnded( event ) )
                                                             { event.setHandled(); }
                                                           } ) );
  
  storeConnection( window->getSignalMouseDown().connect( [this]( app::MouseEvent &event )
                                                        {
                                                          if( deepMouseDown( event ) )
                                                          { event.setHandled(); }
                                                        } ) );
  storeConnection( window->getSignalMouseDrag().connect( [this]( app::MouseEvent &event )
                                                        {
                                                          if( deepMouseDrag( event ) )
                                                          { event.setHandled(); }
                                                        } ) );
  storeConnection( window->getSignalMouseUp().connect( [this]( app::MouseEvent &event )
                                                      {
                                                        if( deepMouseUp( event ) )
                                                        { event.setHandled(); }
                                                      } ) );
}

void RootNode::disconnect()
{
  mConnectionManager.disconnect();
}
