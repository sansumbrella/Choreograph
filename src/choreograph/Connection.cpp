/*
 * Copyright (c) 2014 David Wicks, sansumbrella.com
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

#include "Connection.h"
#include "Output.h"

using namespace std;
using namespace choreograph;

//=================================================
// ConnectionBase
//=================================================

ConnectionBase::~ConnectionBase()
{
  disconnect( _output_base );
}

ConnectionBase::ConnectionBase( void *target ):
_raw_target( target )
{}

ConnectionBase::ConnectionBase( OutputBase *output ):
_raw_target( output ),
_output_base( output )
{
  // Disconnect output from previous parent.
  // No need to compare against this, as this didn't exist before.
  _output_base->disconnect();
  // Tell output about ourselves.
  _output_base->_input = this;
}

void ConnectionBase::disconnect( OutputBase *base )
{
  if( _output_base && _output_base == base ) {
    _output_base->_input = nullptr;
    _output_base = nullptr;
    _raw_target = nullptr;
  }
}

void ConnectionBase::connect( OutputBase *base )
{
  if( _output_base != base ) {
    disconnect( _output_base );

    _output_base = base;
    _raw_target = base;
    _output_base->_input = this;
    // Tell derived class to do the right thing.
    replaceOutput( base );
  }
}
