//
//  BezierEase.h
//
//  Created by Soso Limited on 1/11/16.
//
//

#pragma once
#include "UnitBezier.h"

namespace choreograph {

const auto createBezierEase(double x1, double y1, double x2, double y2) -> std::function<float (float)> {
  auto bezier = BezierInterpolant(x1, y1, x2, y2);
  auto epsilon = std::numeric_limits<float>() * 100;

  return [bezier, epsilon] (float t) {
    return bezier.solve(t, epsilon);
  };
}

} // namespace choreograph
