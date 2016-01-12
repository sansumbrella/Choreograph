//
//  TextureNode.cpp
//  WordShift
//
//  Created by David Wicks on 5/3/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "TextureNode.h"
#include "cinder/gl/draw.h"

using namespace cinder;
using namespace pockets;
using namespace cobweb;

TextureNode::TextureNode( const gl::TextureRef &texture ):
  mTexture( texture )
{}

TextureNode::~TextureNode()
{}

void TextureNode::draw()
{
  gl::draw( mTexture, app::toPoints( mTexture->getBounds() ) );
}
