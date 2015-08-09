//
//  TextureNode.h
//  WordShift
//
//  Created by David Wicks on 5/3/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once
#include "Node.h"
#include "cinder/gl/Texture.h"

namespace pockets
{
  namespace cobweb
  {
  typedef std::shared_ptr<class TextureNode> TextureNodeRef;
  class TextureNode : public Node
  {
  public:
    TextureNode( const ci::gl::TextureRef &texture );
    ~TextureNode();
    void draw();
    static TextureNodeRef create( const ci::gl::TextureRef &texture ){ return TextureNodeRef( new TextureNode( texture ) ); }
  private:
    ci::gl::TextureRef   mTexture;
  };
  } // cobweb::
} // pockets::
