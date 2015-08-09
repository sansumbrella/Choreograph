//
//  SimpleButton.h
//  WordShift
//
//  Created by David Wicks on 2/22/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once
#include "cinder/gl/Texture.h"
#include "Node.h"
#include "ButtonBase.h"
#include "cinder/Font.h"

namespace pockets
{
  namespace cobweb
  {
  typedef std::shared_ptr<class SimpleButton> SimpleButtonRef;
  /**
  A simple button specialized for Word Shift's needs
  Displays a texture on top of a rectangular backing
  Pulls color from the global game color functions based on state.
  */
  class SimpleButton : public ButtonBase
  {
  public:
    ~SimpleButton();
    void                    draw();
    void                    setHitPadding( float horizontal, float vertical );
    void                    hoverStart() override;
    void                    hoverEnd() override;
    //! create a button with a string label foreground set in \a font
    static SimpleButtonRef  createLabelButton( const std::string &str, const ci::Font &font );
    //! create a button from a foreground graphic centered in bounds
    static SimpleButtonRef create( const ci::gl::TextureRef &foreground, const ci::Rectf &bounds );
  private:
    SimpleButton( const ci::gl::TextureRef &foreground, const ci::Rectf &bounds );
    //! screen boundary of element
    ci::Rectf           mBackgroundBounds;
    ci::Rectf           mForegroundBounds;

    ci::gl::TextureRef  mForegroundTexture;
    ci::Color           mBackingColor = ci::Color::black();
    ci::Color           mForegroundColor = ci::Color::white();
  };
  } // cobweb::

} // pockets::
