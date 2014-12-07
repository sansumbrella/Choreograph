//
//  Node.cpp
//  WordShift
//
//  Created by David Wicks on 3/23/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "Node.h"
#include "pockets/CollectionUtilities.hpp"
#include "cinder/gl/gl.h"

using namespace std;
using namespace cinder;
using namespace pockets;
using namespace cobweb;

NodeUniqueRef Node::create()
{
  return NodeUniqueRef( new Node() );
}

Node::~Node()
{
  for( auto &child : mChildren )
  {
    child->mParent = nullptr;
  }
}

void Node::appendChild( NodeRef element )
{
  insertChildAt( element, mChildren.size() );
}

void Node::insertChildAt( NodeRef child, size_t index )
{
  Node *former_parent = child->getParent();
  if( former_parent ) // remove child from parent (but skip notifying child)
  { vector_remove( &former_parent->mChildren, child ); }
  child->setParent( this );
  mChildren.insert( mChildren.begin() + index, child );
  childAdded( child );
}

void Node::setChildIndex(NodeRef child, size_t index)
{
  vector_remove( &mChildren, child );
  index = math<int32_t>::min( index, mChildren.size() );
  mChildren.insert( mChildren.begin() + index, child );
}

void Node::removeChild( NodeRef element )
{
  vector_remove( &mChildren, element );
  element->mParent = nullptr;
}

void Node::removeChild( Node *element )
{
  vector_erase_if( &mChildren, [element]( NodeRef &n ){ return n.get() == element; } );
  element->mParent = nullptr;
}

void Node::setParent( Node *parent )
{
  if( mParent && mParent != parent )
  { mParent->removeChild( this ); }
  mParent = parent;
}

bool Node::deepTouchesBegan( ci::app::TouchEvent &event )
{
  bool captured = touchesBegan( event );
  for( NodeRef &child : mChildren )
  { // stop evaluation if event was captured by self or a child
    if( captured )
      { break; }
    captured = child->deepTouchesBegan( event );
  }
  return captured;
}

bool Node::deepTouchesMoved( ci::app::TouchEvent &event )
{
  bool captured = touchesMoved( event );
  for( NodeRef &child : mChildren )
  { // stop evaluation if event was captured by self or a child
    if( captured )
      { break; }
    captured = child->deepTouchesMoved( event );
  }
  return captured;
}

bool Node::deepTouchesEnded( ci::app::TouchEvent &event )
{
  bool captured = touchesEnded( event );
  for( NodeRef &child : mChildren )
  { // stop evaluation if event was captured by self or a child
    if( captured )
      { break; }
    captured = child->deepTouchesEnded( event );
  }
  return captured;
}

bool Node::deepMouseDown( ci::app::MouseEvent &event )
{
  bool captured = mouseDown( event );
  for( NodeRef &child : mChildren )
  { // stop evaluation if event was captured by self or a child
    if( captured )
      { break; }
    captured = child->deepMouseDown( event );
  }
  return captured;
}

bool Node::deepMouseDrag( ci::app::MouseEvent &event )
{
  bool captured = mouseDrag( event );
  for( NodeRef &child : mChildren )
  { // stop evaluation if event was captured by self or a child
    if( captured )
      { break; }
    captured = child->deepMouseDrag( event );
  }
  return captured;
}

bool Node::deepMouseUp( ci::app::MouseEvent &event )
{
  bool captured = mouseUp( event );
  for( NodeRef &child : mChildren )
  { // stop evaluation if event was captured by self or a child
    if( captured )
      { break; }
    captured = child->deepMouseUp( event );
  }
  return captured;
}

void Node::deepDraw()
{
  gl::ScopedModelMatrix matrix;
  gl::multModelMatrix( mat4( mLocus.toMatrix() ) );

  draw();

  preChildDraw();
  for( NodeRef &child : mChildren ) {
    child->deepDraw();
  }
  postChildDraw();
}

void Node::deepCancelInteractions()
{
  cancelInteractions();
  for( NodeRef &child : mChildren ) {
    child->deepCancelInteractions();
  }
}

mat4 Node::getFullTransform() const
{
  mat4 mat = mLocus.toMatrix();
  if( mParent )
    { mat = mParent->getFullTransform() * mat; }
  return mat;
}
