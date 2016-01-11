//
//  Channel_testTest.cpp
//
//  Created by Soso Limited on 1/11/16.
//
//

#include "catch.hpp"
#include "Channel.hpp"

using namespace choreograph;

TEST_CASE("Channel")
{

  SECTION("Simple channel interpolation")
  {
    auto channel = Channel<float>();
    channel.appendKeyAfter(0.0f, 0)
    .appendKeyAfter(10.0f, 1.0)
    .appendKeyAfter(5.0f, 1.0);

    REQUIRE(channel.index(-0.5) == 0);
    REQUIRE(channel.index(0.5) == 0);
    REQUIRE(channel.index(1.0) == 0);
    REQUIRE(channel.index(1.1) == 1);
    REQUIRE(channel.index(5.5) == 1);
  }

  SECTION("Default bezier handles result in effectively linear interpolation.")
  {
    auto bezier = BezierInterpolant();

    for (auto t = 0.0f; t <= 1.0f; t += 0.05f) {
      CHECK(bezier.curveX(t) == Approx(t));
      CHECK(bezier.curveY(t) == Approx(t));
    }

    auto solution = bezier.solve(0.5f, std::numeric_limits<float>::epsilon());
    REQUIRE(solution == Approx(0.5));
  }

}
