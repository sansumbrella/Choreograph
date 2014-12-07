//
//  ButtonBase.cpp
//  WordShift
//
//  Created by David Wicks on 4/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "ButtonBase.h"

using namespace cinder;
using namespace pockets;
using namespace cobweb;

ButtonBase::ButtonBase( const Rectf &bounds ):
mHitBounds( bounds )
{}

ButtonBase::~ButtonBase()
{}

void ButtonBase::expandHitBounds( float horizontal, float vertical )
{
  mHitBounds.x1 -= horizontal;
  mHitBounds.x2 += horizontal;
  mHitBounds.y1 -= vertical;
  mHitBounds.y2 += vertical;
}

void ButtonBase::cancelInteractions()
{
  mTrackedTouch = 0;
  endHovering( false );
}

void ButtonBase::endHovering( bool selected )
{
  if( mHovering )
  {
    mHovering = false;
    hoverEnd();
  }
}

void ButtonBase::setHovering()
{
  if( !mHovering )
  {
    mHovering = true;
    hoverStart();
  }
}

bool ButtonBase::mouseDown(ci::app::MouseEvent &event)
{
  if( contains( event.getPos() ) )
  {
    mTrackedTouch = std::numeric_limits<uint32_t>::max();
    setHovering();
    return true;
  }
  return false;
}

bool ButtonBase::mouseDrag(ci::app::MouseEvent &event)
{
  if( contains( event.getPos() ) )
  {
    setHovering();
  }
  else
  {
    endHovering( false );
  }
  return false;
}

bool ButtonBase::mouseUp(ci::app::MouseEvent &event)
{
  bool selected = false;
  mTrackedTouch = 0;
  if( contains( event.getPos() ) )
    { selected = true; }
  endHovering( selected );
  if( selected )
    { emitSelect(); }
  return selected;
}

bool ButtonBase::touchesBegan(ci::app::TouchEvent &event)
{
  for( auto &touch : event.getTouches() )
  {
    if( contains( touch.getPos() ) )
    {
      mTrackedTouch = touch.getId();
      setHovering();
      return true;
    }
  }
  return false;
}

bool ButtonBase::touchesMoved(ci::app::TouchEvent &event)
{
  for( auto &touch : event.getTouches() )
  {
    if( touch.getId() == mTrackedTouch )
    { // sliding a finger into a button won't trigger it
      if( contains( touch.getPos() ) )
        { setHovering(); }
      else
        { endHovering( false ); }
    }
  }
  return false;
}

bool ButtonBase::touchesEnded(ci::app::TouchEvent &event)
{
  bool selected = false;
  for( auto &touch : event.getTouches() )
  {
    if( touch.getId() == mTrackedTouch )
    {
      mTrackedTouch = 0;
      if( contains( touch.getPos() ) )
      {
        selected = true;
        break;
      }
      endHovering( selected );  // only end hovering if it was our tracked touch that ended
    }
  }
  if( selected )
  { // in case of side effects in select function, emit selection last
    // e.g. if button navigates to a new screen, it will destroy itself
    emitSelect();
  }
  return selected;
}
