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

    SECTION("Index calculating returns the curve index at time.")
    {
      REQUIRE(channel.index(-0.5) == 0);
      REQUIRE(channel.index(0.6) == 0);
      REQUIRE(channel.index(1.0) == 0);
      REQUIRE(channel.index(1.1) == 1);
      REQUIRE(channel.index(5.5) == 1);
    }

    SECTION("Inserting into channel")
    {
      channel.insertKey(0.5f, 0.5);
      REQUIRE(channel.index(0.6) == 1);
      REQUIRE(channel.value(0.5) == 0.5);
      REQUIRE(channel.value(0.25) == Approx(0.25));
    }

    SECTION("Value calculation is roughly linear by default.")
    {
      REQUIRE(channel.value(0.5) == Approx(5.0f)); // 0.5 is not exact enough, though it still prints as 0.5.
      REQUIRE(channel.value(1.5) == 7.5f);
    }

    SECTION("Out of bound values are clamped to ends of range.")
    {
      REQUIRE(channel.value(2.2) == 5.0f);
      REQUIRE(channel.value(-0.1) == 0.0f);
      REQUIRE(channel.value(0.0) == 0.0f);
    }
  }

  SECTION("Default bezier handles result in effectively linear interpolation.")
  {
    auto bezier = BezierInterpolant();

    for (auto t = 0.0f; t <= 1.0f; t += 0.05f) {
      CHECK(bezier.curveX(t) == Approx(t));
      CHECK(bezier.curveY(t) == Approx(t));
    }

    auto solution = bezier.solve(0.5f);
    REQUIRE(solution == Approx(0.5));
  }

}
