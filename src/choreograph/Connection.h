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
#include "Output.h"

namespace choreograph
{

///
/// Connection maintains a connection to an output.
/// Manages lifecycle of connection along with Output<T>.
/// If connected to a raw pointer, no lifecycle management occurs.
///

template<typename T>
class Connection
{
public:
  /// Constructs and invalid Connection.
  Connection() = default;

  ~Connection();

  /// Create a Connection to a managed Output pointer.
  /// Connection will be disconnected when either Connection or Output falls out of scope.
  explicit Connection( Output<T> *base );

  /// Create a Connection to a raw pointer.
  /// No lifetime management will occur.
  /// In most cases, use Connection( Output<T>* ) instead of this.
  explicit Connection( T *target );

  /// Returns true iff this Connection has an output.
  bool  isConnected() const { return _raw_target != nullptr; }

  /// Returns raw pointer to target variable. Used for comparison.
  const T* targetPtr() const { return _raw_target; }

  /// Returns reference to target variable. Used for assignment.
  T& target() { return *_raw_target; }

private:
  // void pointer to target, used for comparison with other MotionBase's.
  T          *_raw_target = nullptr;
  // Pointer to safe handle type. Exists iff created with an OutputBase target.
  Output<T>  *_output_base = nullptr;

  /// Remove connection to Output.
  /// Called on destruction of either this or _output_base.
  void disconnect( Output<T> *base );

  /// Connect to a new Output (or update the pointer if Output's address changed).
  /// Called by OutputBase when one Output supplants another (e.g. in copy-construction, move assignment, etc).
  void connect( Output<T> *base );

  friend class Output<T>;
};

//=================================================
// Connection Implementation
//=================================================

template<typename T>
Connection<T>::Connection( Output<T> *base ):
  _output_base( base ),
  _raw_target( base->valuePtr() )
{
  _output_base->disconnect();
  _output_base->_input = this;
}

template<typename T>
Connection<T>::Connection( T *target ):
  _raw_target( target )
{}

template<typename T>
Connection<T>::~Connection()
{
  disconnect( _output_base );
}

template<typename T>
void Connection<T>::disconnect( Output<T> *base )
{
  if( _output_base && _output_base == base ) {
    _output_base->_input = nullptr;
    _output_base = nullptr;
    _raw_target = nullptr;
  }
}

template<typename T>
void Connection<T>::connect( Output<T> *base )
{
  if( _output_base != base ) {
    disconnect( _output_base );

    _output_base = base;
    _raw_target = base->valuePtr();
    _output_base->_input = this;
  }
}

template<typename T>
using ConnectionRef = std::shared_ptr<Connection<T>>;

}
