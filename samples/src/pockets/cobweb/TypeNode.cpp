//
//  TypeNode.cpp
//  WordShift
//
//  Created by David Wicks on 6/15/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "TypeNode.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/gl/gl.h"

using namespace std;
using namespace cinder;
using namespace pockets;
using namespace cobweb;

TypeNode::TypeNode( gl::TextureFontRef font, const std::string &text, bool flipped ):
  mFont( font ),
  mFlipped( flipped )
{
  gl::TextureFont::DrawOptions opt;
  opt.scale( 1.0f / app::getWindow()->getContentScale() ).pixelSnap( false );
  mGlyphs = mFont->getGlyphPlacements( text, opt );
}

TypeNode::~TypeNode()
{}

void TypeNode::setText( const string &text )
{
  gl::TextureFont::DrawOptions opt;
  opt.scale( 1.0f / app::getWindow()->getContentScale() ).pixelSnap( false );
  mGlyphs = mFont->getGlyphPlacements( text, opt );
}

void TypeNode::draw()
{
  if( mFlipped ) {
    gl::scale( 1.0f, -1.0f );
  }

  mFont->drawGlyphs( mGlyphs, vec2( 0 ) );
}

TypeNodeUniqueRef TypeNode::create( gl::TextureFontRef font, const std::string &text )
{
  return TypeNodeUniqueRef( new TypeNode( font, text ) );
}
