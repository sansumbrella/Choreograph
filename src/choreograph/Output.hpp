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

namespace choreograph
{

template<typename T> class Motion;

///
/// Safe type for Choreograph outputs.
/// Disconnects applied Motion on destruction so you don't accidentally modify stale pointers.
///
template<typename T>
class Output
{
public:
  /// Disconnect on destruction.
  ~Output() { disconnect(); }

  /// Default constructor.
  Output() = default;

  /// Construct with a value.
  Output( const T &value ):
  _value( value )
  {}

  /// Move assignment takes value and any input Motion.
  Output<T>& operator= ( Output<T> &&rhs );
  /// Move constructor takes value and any input Motion.
  Output( Output<T> &&rhs );

  /// Delete copy assignment. Behavior was confusing due to relationship with Connection<T>.
  Output<T>& operator= ( const Output<T> &rhs ) = delete;
  /// Delete copy constructor. Behavior was confusing due to relationship with Connection<T>.
  Output( const Output<T> &rhs ) = delete;

  /// Disconnect from Motion input.
  void disconnect();

  /// Returns true iff this Output has a Connection input.
  bool isConnected() const { return _input != nullptr; }

  /// Value assignment operator.
  Output<T>& operator= ( T value ) { _value = value; return *this; }
  /// Value add-assign.
  Output<T>& operator+= ( T value ) { _value += value; return *this; }

  /// Returns value of output.
  const T& 	value() const { return _value; }

  /// Returns value of output.
  const T& 	operator() () const { return _value; }

  /// Returns value of output for manipulating.
  T&				operator() () { return _value; }

  /// Enable cast to value type.
  operator const T&()	{ return _value; }

  /// Returns pointer to value.
  const T* 	valuePtr() const { return &_value; }

  /// Returns pointer to value.
  T*				valuePtr() { return &_value; }

private:
  T         _value;
  Motion<T> *_input = nullptr;

  /// Replaces \a rhs in its relationship to a TimelineItem input.
  void supplant( Output<T> &&rhs );
  /// Connects this output to a different Connection.
  void connect( Connection<T> *input );

  friend class Motion<T>;
};

//=================================================
// Output Implementation
//=================================================

// Move constructor takes value and any input Motion.
template<typename T>
Output<T>::Output( Output<T> &&rhs ):
  _value( std::move( rhs._value ) ),
  _input( std::move( rhs._input ) )
{
  if( _input ) {
    _input->setOutput( this );
  }
}

// Move assignment takes value and any input Motion.
template<typename T>
Output<T>& Output<T>::operator= ( Output<T> &&rhs ) {
  if( this != &rhs ) {
    _value = std::move( rhs._value );
    _input = std::move( rhs._input );
    if( _input ) {
      _input->setOutput( this );
    }
  }
  return *this;
}

template<typename T>
void Output<T>::disconnect()
{
  if( _input ) {
    _input->cancel();
    _input = nullptr;
  }
}

} // namespace choreograph
