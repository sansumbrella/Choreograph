/*
 * Copyright (c) 2013 David Wicks
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

#pragma once

/** \file
 Functions for easier manipulation of STL containers.
*/

#include "Pockets.h"
#include <map>

namespace pockets
{
	//! Remove all elements from \a map for which \a compare returns true
	template<class MAP_TYPE, class COMPARATOR>
	void map_erase_if( MAP_TYPE *map, COMPARATOR compare )
	{
		auto iter = map->begin();
		while( iter != map->end() )
		{
			if( compare( iter->second ) )
			{
				map->erase( iter++ );
			}
			else
			{
				iter++;
			}
		}
	}

  //! Return a vector of all the keys in a map
  template<typename K, typename V>
  std::vector<K> map_keys( const std::map<K, V> &map )
  {
    std::vector<K> ret;
    for( auto &pair : map )
    {
      ret.push_back( pair.first );
    }
    return ret;
  }

  //! Remove all elements from \a vec that match \a compare
  template<class ELEMENT_TYPE, class COMPARATOR>
  void vector_erase_if( std::vector<ELEMENT_TYPE> *vec, COMPARATOR compare )
  {
    vec->erase( std::remove_if( vec->begin()
                              , vec->end()
                              , compare )
               , vec->end() );
  }

  //! Remove all elements from \a container that match \a compare
  //! This is closer to an earlier strategy I had than vector_erase_if,
  //! but that was plagued by obscure error messages. Will see if this works
  //! a bit better / more flexibly
  template<class CONTAINER_TYPE, class COMPARATOR>
  void erase_if( CONTAINER_TYPE *container, COMPARATOR compare )
  {
    container->erase( std::remove_if( container->begin(),
                                      container->end(),
                                      compare ),
                     container->end() );
  }

  //! Remove all copies of \a element from \a vec
  template<class ELEMENT_TYPE>
  void vector_remove( std::vector<ELEMENT_TYPE> *vec, const ELEMENT_TYPE &element )
  {
    vec->erase( std::remove_if( vec->begin()
                               , vec->end()
                               , [=](const ELEMENT_TYPE &e){ return e == element; } )
               , vec->end() );
  }

  //! Returns true if \a vec contains the element \a compare
  template<class ELEMENT_TYPE>
  bool vector_contains( const std::vector<ELEMENT_TYPE> &vec, const ELEMENT_TYPE &compare )
  {
    return std::find( vec.begin(), vec.end(), compare ) != vec.end();
  }

  //! Returns true if \a compare function returns true for an element in \a vec
  template<class ELEMENT_TYPE, class COMPARATOR>
  bool vector_contains( const std::vector<ELEMENT_TYPE> &vec, COMPARATOR compare )
  {
    return std::find_if( vec.begin(), vec.end(), compare ) != vec.end();
  }
}
