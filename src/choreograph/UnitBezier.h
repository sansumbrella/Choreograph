/*
 * Copyright (c) 2016 David Wicks, sansumbrella.com
 * All rights reserved.
 *
 * Bezier math is taken from Webkit's UnitBezier.
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
#include <limits>

namespace choreograph {

/// Some bezier control values.
/// May get smarter in future, with reference to host channel so you can walk the animation.
/// But perhaps that would be a separate convenience class, so we only store bezier values.
/// May be templated in future, so a channel could have separate interpolation across components (e.g. a vec3 channel with xyz easing separately).
/// Implicit first and last control points are (0,0) and (1,1).
class BezierInterpolant {
public:
  BezierInterpolant():
    _x1(0.3333333), _y1(0.3333333), _x2(0.6666666), _y2(0.6666666)
  {
    calculateXCoefficients();
    calculateYCoefficients();
  }
  BezierInterpolant(double x1, double y1, double x2, double y2):
    _x1(x1),
    _y1(y1),
    _x2(x2),
    _y2(y2)
  {
    calculateXCoefficients();
    calculateYCoefficients();
  }

  /// Given a value x, solve for y on the curve.
  double solve(double x, double epsilon = std::numeric_limits<float>::epsilon() * 100.0f) const {
    return curveY(timeAtX(x, epsilon));
  }

  double curveX(double t) const {
    // at^3 + bt^2 + ct expanded using Horner's rule.
    return ((ax * t + bx) * t + cx) * t;
  }

  double curveY(double t) const {
    return ((ay * t + by) * t + cy) * t;
  }

  double derivativeX(double t) const {
    // 3at^2 + 2bt + c
    return (3 * ax * t + 2 * bx) * t + cx;
  }

  /// Given an x value, find a parameter t that generates it.
  double timeAtX(double x, double epsilon) const;

private:
  double _x1, _y1;
  double _x2, _y2;
  double ax, bx, cx;
  double ay, by, cy;

  void calculateXCoefficients() {
    cx = 3 * _x1;
    bx = 3 * (_x2 - _x1) - cx;
    ax = 1 - cx - bx;
  }
  void calculateYCoefficients() {
    cy = 3 * _y1;
    by = 3 * (_y2 - _y1) - cy;
    ay = 1 - cy - by;
  }
};

} // namespace choreograph
