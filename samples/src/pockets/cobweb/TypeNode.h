//
//  TypeNode.h
//  WordShift
//
//  Created by David Wicks on 6/15/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "Node.h"

namespace cinder
{
  namespace gl
  {
    // forward decl
    typedef std::shared_ptr<class TextureFont> TextureFontRef;
  }
}

namespace pockets
{
  namespace cobweb
  {
  typedef std::shared_ptr<class TypeNode> TypeNodeRef;
  typedef std::unique_ptr<class TypeNode> TypeNodeUniqueRef;
  class TypeNode : public Node
  {
  public:
    TypeNode( ci::gl::TextureFontRef font, const std::string &text, bool flipped=false );
    ~TypeNode();
    static TypeNodeUniqueRef create( ci::gl::TextureFontRef font, const std::string &text);
    void draw();
	void setText( const std::string &text );
  private:
    ci::gl::TextureFontRef							mFont;
    std::vector< std::pair< uint16_t, ci::vec2 > > mGlyphs;
	bool											mFlipped;
  };
  } // cobweb::
} // pockets::
