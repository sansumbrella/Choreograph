//
//  ButtonBase.h
//  WordShift
//
//  Created by David Wicks on 4/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "Node.h"


namespace pockets
{
  namespace cobweb
  {
  typedef std::shared_ptr<class ButtonBase> ButtonRef;
  /**
   Handler of touch events to fire off button "presses"
   Doesn't render itself

   Subclasses are responsible for drawing themselves to screen.
   Would be cool to have a button decorator to turn anything into a button
  */
  class ButtonBase : public Node
  {
  public:
    typedef std::function<void ()> SelectFn;
    ButtonBase( const ci::Rectf &bounds );
    virtual ~ButtonBase();
    //! returns hit box size in points
    ci::Rectf     	getHitBounds() const { return mHitBounds; }
    //!
    ci::vec2       getSize() const { return ci::vec2( getWidth(), getHeight() ); }
    float           getHeight() const { return mHitBounds.getHeight(); }
    float           getWidth() const { return mHitBounds.getWidth(); }
    //! set the hit bounds in points
    void            setHitBounds( const ci::Rectf &bounds ){ mHitBounds = bounds; }
    //! expand hit box by \a horizontal and \a vertical pixels on each side
    void            expandHitBounds( float horizontal, float vertical );
    //! set the function to call when this button is selected or "pressed"
    void            setSelectFn( SelectFn fn ){ mSelectFn = fn; }
    //! called on touches end if this button was selected
    void            emitSelect() { if( mSelectFn ){ mSelectFn(); } }
    //! stop tracking the touch
    void            cancelInteractions();
    //! called when a finger enters the button's hit area
    virtual void    hoverStart() {}
    //! called when a finger leaves the button's hit area
    virtual void    hoverEnd() {}
  protected:
    // handle Node interaction events
    virtual bool    touchesBegan( ci::app::TouchEvent &event ) override;
    virtual bool    touchesMoved( ci::app::TouchEvent &event ) override;
    virtual bool    touchesEnded( ci::app::TouchEvent &event ) override;
    virtual bool    mouseDown( ci::app::MouseEvent &event ) override;
    virtual bool    mouseDrag( ci::app::MouseEvent &event ) override;
    virtual bool    mouseUp( ci::app::MouseEvent &event ) override;
  private:
    //! touch boundary of element
    ci::Rectf       mHitBounds;
    uint32_t        mTrackedTouch = 0;
    SelectFn        mSelectFn;
    bool            mHovering = false;

    void            setHovering();
    void            endHovering( bool selected );

    //! returns whether a point is inside the button's hit box
    bool            contains( const ci::ivec2 point ) { return mHitBounds.contains( transformedPoint(point) ); }
    //! unproject point from our drawing space
    ci::vec2       transformedPoint( const ci::vec2 &point )
    {
      return ci::vec2( ci::inverse( getFullTransform() ) * ci::vec4( point, 0.0f, 1.0f ) );
    }
  };
  } // cobweb::
} // pockets::
