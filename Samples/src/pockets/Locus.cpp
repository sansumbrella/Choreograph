/*
 * Copyright (c) 2013 David Wicks, sansumbrella.com
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

#include "pockets/Locus.h"

using namespace pockets;
using namespace cinder;

mat4 Locus2D::toMatrix() const
{
  mat4 mat;
  mat = translate( mat, vec3( position + registration_point, 0.0f ) );
  mat = rotate( mat, rotation, vec3( 0.0f, 0.0f, 1.0f ) );
  mat = ci::scale( mat, vec3( scale, 1.0f ) );
  mat = translate( mat, vec3( -registration_point, 0.0f ) );
  if( parent ){ mat = parent->toMatrix() * mat; }
  return mat;
}

ci::vec2 Locus2D::worldScale() const
{
  return parent ? parent->worldScale() * scale : scale;
}

float Locus2D::worldRotation() const
{
  return parent ? parent->worldRotation() + rotation : rotation;
}

vec2 Locus2D::worldPosition() const
{
  return parent ? vec2(parent->toMatrix() * vec4(position, 0.0f, 1.0f)) : position;
}

void Locus2D::detachFromParent()
{
  if( parent )
  {
    scale *= parent->worldScale();
    rotation += parent->worldRotation();
    position = vec2(parent->toMatrix() * vec4(position, 0.0f, 1.0f));

    parent.reset();
  }
}
