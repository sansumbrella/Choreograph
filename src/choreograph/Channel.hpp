/*
 * Copyright (c) 2014—2016 David Wicks, sansumbrella.com
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
#include <vector>
#include "Phrase.hpp"
#include "UnitBezier.h"
#include "TimeType.h"

namespace choreograph
{

class Curve
{
public:
  enum Type {
    Bezier,
    Hold,
    Linear
  };

  Curve() = default;
  explicit Curve(Type t):
    _type(t)
  {}
  explicit Curve(const BezierInterpolant &bezier):
    _type(Bezier),
    _bezier(bezier)
  {}

  float              solve(float t) const;
  void               setType(Type type) { _type = type; }
  void               hold() { _type = Hold; }
  BezierInterpolant& bezier() { _type = Bezier; return _bezier; }
  Type               type() const { return _type; }
private:
  Type              _type = Linear;
  BezierInterpolant _bezier;
};

inline float Curve::solve(float t) const
{
  switch (_type) {
    case Bezier:
      return _bezier.solve(t);
    break;
    case Hold:
      return 0.0f;
    break;
    case Linear:
      return t;
    break;
    default:
      return t;
    break;
  }
}

///
/// Simple channel concept with bezier interpolation between keys.
/// Goals:
/// - Easy serialization
/// - Easy to create graphical manipulation tools.
///   - Support direct manipulation of animation data.
///   - Avoid deeply nested information. Flat hierarchy.
/// - Flexible enough to create an AfterEffects-style timeline.
///   - Separate motion path with bezier interpolation for full effect.
///   - Get time along section with channel, calculate value using path.
///   - Channel for timing, with key values corresponding to control points on spatial curve.
///
/// Good for animating single values, like floats or quaternions.
/// For vector types, use a channel per component (and maybe add a group type to associate them).
/// Grouping type could be a manipulator/animator type class that reads/writes multiple channels.
///
template <typename T>
class Channel
{
public:
  struct Key {
    Key(T value, Time time):
      value(value),
      time(time)
    {}
    T    value;
    Time time;
  };

  class KeyInfo
  {
  public:
    KeyInfo(Key &current, Key *previous, Key *next, Curve *c1, Curve *c2):
      _current(current),
      _previous(previous),
      _next(next),
      _curve_in(c1),
      _curve_out(c2)
    {}

    T    value() const {
      return _current.value;
    }
    T    previousValue() const {
      return _previous ? _previous->value : value();
    }
    T    nextValue() const {
      return _next ? _next->value : value();
    }
    Time time() const {
      return _current.time;
    }
    Time nextTime() const {
      return _next ? _next->time : std::numeric_limits<Time>::max();
    }
    Time previousTime() const {
      return _previous ? _previous->time : 0;
    }

    bool isFirst() const { return _previous == nullptr; }
    bool isLast() const { return _next == nullptr; }

    const Curve* curveOut() const { return _curve_out; }
    const Curve* curveIn() const { return _curve_in; }

  protected:
    Key   &_current;
    Curve *_curve_in = nullptr;
    Curve *_curve_out = nullptr;
  private:
    Key   *_previous = nullptr;
    Key   *_next = nullptr;
  };

  class KeyManipulator : public KeyInfo
  {
  public:
    KeyManipulator(Key &current, Key *previous, Key *next, Curve *c1, Curve *c2):
      KeyInfo(current, previous, next, c1, c2)
    {}

    void setValue(const T &value) {
      this->_current.value = value;
    }

    void setTime(Time t) {
      this->_current.time = std::min(std::max(t, this->previousTime()), this->nextTime());
    }

    Curve* curveOut() { return this->_curve_out; }
    Curve* curveIn() { return this->_curve_in; }
  };

  Channel() = default;
  template <typename ... Keys>
  Channel(T value, Time at_time, Curve::Type curve_type, Keys&&... keys) {
    insertKey(value, at_time, curve_type, std::forward<Keys>(keys)...);
  }

  /// Return the value of the channel at a given time.
  T value(Time at_time) const;
  /// Return the interpolated value between two keys.
  /// If you keep track of keys yourself, this is faster.
  T interpolatedValue(size_t curve_index, Time at_time) const;

  /// Returns the index of the starting key for the current time.
  /// The curve lies between two keys.
  size_t index(Time at_time) const;
  size_t lastIndex() const { return _keys.empty() ? 0 : _keys.size() - 1; }

  /// Append a key to the list of keys, positioned at \offset time after the previous key.
  Channel& appendKeyAfter(T value, Time offset, Curve::Type curve_type = Curve::Linear) {
    _keys.emplace_back(value, duration() + offset);
    _curves.emplace_back(curve_type);
    return *this;
  }
  /// Insert a key at the given time.
  Channel& insertKey(T value, Time at_time, Curve::Type curve_type = Curve::Linear);
  /// Insert multiple keys.
  template <typename ... Keys>
  Channel& insertKey(T value, Time at_time, Curve::Type curve_type, Keys&&... keys);

  bool     empty() const { return _keys.empty(); }
  Time     duration() const { return empty() ? 0 : _keys.back().time; }
  const std::vector<Key>&   keys() const { return _keys; }
  const std::vector<Curve>& curves() const { return _curves; }

  /// Returns a manipulator to adjust properties of the requested key.
  KeyManipulator keyControl(size_t key_index);
  /// Returns an object allowing easier observation of a key and its neighbors.
  const KeyInfo  keyInfo(size_t key_index) const;

  // TODO: remove general mutability from keys/curves in favor of KeyManipulators.
  std::vector<Key>&   mutableKeys() { return _keys; }
  std::vector<Curve>& mutableCurves() { return _curves; }

private:
  std::vector<Key>   _keys;
  // may make polymorphic in future (like phrases are). Basically just an ease fn.
  // Compare std::function<float (float)> for max flexibility against custom class.
  std::vector<Curve> _curves;
};

#pragma mark - Channel Template Implementation

template <typename T>
size_t Channel<T>::index(Time at_time) const {

  if( at_time <= 0 ) {
    return 0;
  }
  else if ( at_time >= this->duration() ) {
    return lastIndex();
  }

  for (auto i = 0; i < _keys.size() - 1; i += 1) {
    auto &a = _keys[i], &b = _keys[i + 1];
    if (a.time <= at_time && b.time > at_time) {
      return i;
    }
  }
  return lastIndex();
}

template <typename T>
T Channel<T>::value(Time at_time) const {
  if (at_time >= duration()) {
    return _keys.back().value;
  }
  else if (at_time <= 0.0) {
    return _keys.front().value;
  }

  return interpolatedValue(index(at_time), at_time);
}

template <typename T>
T Channel<T>::interpolatedValue(size_t curve_index, Time at_time) const {
  auto &a = _keys[curve_index];
  auto &b = _keys[curve_index + 1];
  auto &c = _curves[curve_index];

  auto x = (at_time - a.time) / (b.time - a.time);
  auto t = c.solve(x);
  return lerpT(a.value, b.value, t);
}

template <typename T>
Channel<T>& Channel<T>::insertKey(T value, Time at_time, Curve::Type curve_type) {
  auto i = index(at_time);
  if (_keys.empty()) {
    _keys.emplace_back(value, at_time);
  }
  else {
    _keys.insert(_keys.begin() + i + 1, {value, at_time});
  }

  if (_curves.empty()) {
    _curves.emplace_back(curve_type);
  }
  else {
    _curves.insert(_curves.begin() + i + 1, Curve(curve_type));
  }

  return *this;
}

template <typename T>
template <typename ... Keys>
Channel<T>& Channel<T>::insertKey(T value, Time at_time, Curve::Type curve_type, Keys&&... keys)
{
    insertKey(value, at_time, curve_type);
    insertKey(std::forward<Keys>(keys)...);
    return *this;
}

template <typename T>
typename Channel<T>::KeyManipulator Channel<T>::keyControl(size_t desired_index) {
  auto index = std::min(desired_index, _keys.size() - 1);
  auto &current = _keys[index];
  auto *previous = (index > 0) ? &_keys[index - 1] : nullptr;
  auto *next = (index < _keys.size() - 1) ? &_keys[index + 1] : nullptr;
  auto *c1 = (index > 0) ? &_curves[index - 1] : nullptr;
  auto *c2 = (index < _curves.size()) ? &_curves[index] : nullptr;
  return KeyManipulator(current, previous, next, c1, c2);
}

template <typename T>
const typename Channel<T>::KeyInfo Channel<T>::keyInfo(size_t desired_index) const
{
  auto index = std::min(desired_index, _keys.size() - 1);
  auto &current = const_cast<Key&>(_keys[index]);
  auto *previous = (index > 0) ? &const_cast<Key&>(_keys[index - 1]) : nullptr;
  auto *next = (index < _keys.size() - 1) ? &const_cast<Key&>(_keys[index + 1]) : nullptr;
  auto *c1 = (index > 0) ? &const_cast<Curve&>(_curves[index - 1]) : nullptr;
  auto *c2 = (index < _curves.size()) ? &const_cast<Curve&>(_curves[index]) : nullptr;
  return KeyInfo(current, previous, next, c1, c2);
}

} // namepsace choreograph
