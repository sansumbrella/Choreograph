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

#pragma once

#include "Pockets.h"
#include "cinder/Function.h"
#include "cinder/Signals.h"

namespace pockets
{
/**
 ConnectionManager:
 A utility for managing groups of ci::signals together.
 Helps avoid bloat of connection objects within a class.
 Enables easy pausing, resuming, and destruction of connections.
 Disconnects in destructor to avoid bad access errors.
 */
class ConnectionManager
{
public:
  ConnectionManager() = default;
  /// disconnects all stored connections
  ~ConnectionManager();
  /// store a signal in manager
  inline void store( const ci::signals::Connection &connection )
  { mConnections.push_back( connection ); }
  /// disconnect all stored connections permanently
  /// use block to temporarily block connections
  void        disconnect();

  /// temporarily stop receiving signals
  void        block();
  /// resume receiving blocked signals
  void        resume();

private:
  std::vector<ci::signals::Connection>              mConnections;
};

} // namespace pockets
