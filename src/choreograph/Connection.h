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

#pragma once

#include <memory>

namespace choreograph
{

class OutputBase;
using ConnectionBaseRef = std::shared_ptr<class ConnectionBase>;

///
/// ConnectionBase is the foundation of a connection to an output.
/// Manages lifecycle of connection along with OutputBase.
/// If connected to a raw pointer, no lifecycle management occurs.
/// TODO: consider CRTP for stronger compile-time safety guarantees.
///

class ConnectionBase
{
public:
  /// Constructs and invalid Connection.
  ConnectionBase() = default;

  virtual ~ConnectionBase();

  /// Create a Connection to a managed Output pointer. Preferred use.
  explicit ConnectionBase( OutputBase *base );

  /// Create a Connection to a raw pointer. Not recommended.
  explicit ConnectionBase( void *target );


  /// Returns true iff this Connection has an output.
  bool  isConnected() const { return _raw_target != nullptr; }

  /// Returns raw pointer to target variable. Used for comparison.
  void* getTarget() const { return _raw_target; }

protected:
  /// Replace current output target with new target. Called from connect.
  /// Use to cast new output pointer to correct type.
  virtual void replaceOutput( OutputBase *output ) {}

private:
  // void pointer to target, used for comparison with other MotionBase's.
  void        *_raw_target = nullptr;
  // Pointer to safe handle type. Exists iff created with an OutputBase target.
  OutputBase  *_output_base = nullptr;

  /// Remove connection to Output.
  /// Called on destruction of either this or _output_base.
  void disconnect( OutputBase *base );
  /// Connect to a new Output (or just update the pointer to the same outputbase).
  /// Called by OutputBase when one Output supplants another (e.g. in copy-construction, move assignment, etc).
  void connect( OutputBase *base );
  
  friend class OutputBase;
};
}
