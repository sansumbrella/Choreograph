//
//  Channel_testTest.cpp
//
//  Created by Soso Limited on 1/11/16.
//
//

#include "catch.hpp"
#include "Channel.hpp"

using namespace choreograph;

TEST_CASE("Channel Creation")
{
  SECTION("Channels can be created in a single line")
  {
    auto channel = Channel<float>{
      0.0f, 0.0, Curve::Hold,
      10.0f, 1.0, Curve::Linear,
      20.0f, 2.0
    };

    CHECK(channel.value(1.0) == 10.0f);
    CHECK(channel.value(0.5) == 0.0f);
    CHECK(channel.value(1.5) == 15.0f);
  }
}

TEST_CASE("Channel")
{

  SECTION("Simple channel interpolation")
  {
    auto channel = Channel<float>();
    channel.appendKeyAfter(0.0f, 0)
      .appendKeyAfter(10.0f, 1.0)
      .appendKeyAfter(5.0f, 1.0);

    SECTION("Channels always have one curve per key, which is used to interpolate to the following key (if any).")
    {
      REQUIRE(channel.keys().size() == 3);
      REQUIRE(channel.curves().size() == channel.keys().size());

      channel.insertKey(500.0f, 2.0);
      REQUIRE(channel.curves().size() == channel.keys().size());
    }

    SECTION("Index calculating returns the curve index at time.")
    {
      REQUIRE(channel.index(-0.5) == 0);
      REQUIRE(channel.index(0.6) == 0);
      REQUIRE(channel.index(1.0) == 1);
      REQUIRE(channel.index(1.1) == 1);
      REQUIRE(channel.index(5.5) == 2);
    }

    SECTION("Keys can be inserted at specific times.")
    {
      auto channel = Channel<float>();
      channel.insertKey(10.0f, 0);
      channel.insertKey(20.0f, 1);
      channel.insertKey(30.0f, 3);

      REQUIRE(channel.duration() == 3);
    }

    SECTION("Keys inserted into channels")
    {
      channel.insertKey(0.5f, 0.5);
      REQUIRE(channel.index(0.6) == 1);
      REQUIRE(channel.value(0.5) == 0.5);
      REQUIRE(channel.value(0.25) == Approx(0.25));
      REQUIRE(channel.value(1.5) == 7.5f);
    }

    SECTION("Inserting a key between two keys calculates a split between them.")
    {
      // Will make adding keys to tweak transition between values easier.
      // auto &key = channel.insertKey(0.5);
      // key.curveIn();
      // key.curveOut();
    }

    SECTION("Value calculation is linear by default.")
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

    SECTION("Channels created only by inserting work.")
    {
      auto c = Channel<float>();
      c.insertKey(10.0f, 0);
      c.insertKey(120.0f, 1);
      c.insertKey(30.0f, 3);

      REQUIRE(c.curves().size() == 3);
      REQUIRE(c.value(2) == 75);
    }

    SECTION("Channels can be safely manipulated using KeyManipulators")
    {
      REQUIRE(channel.value(1) == 10.0f);
      auto ctl = channel.keyControl(1);
      ctl.setValue(50.0f);
      REQUIRE(channel.value(1) == 50.0f);
    }
  }

  SECTION("Channel key control")
  {
    auto c = Channel<float>();
    c.insertKey(10.0f, 0);
    c.appendKeyAfter(20.0f, 0.5);
    c.insertKey(120.0f, 1);
    c.insertKey(30.0f, 3);

    CHECK(c.keyControl(0).isFirst());
    CHECK_FALSE(c.keyControl(1).isFirst());
    CHECK_FALSE(c.keyControl(c.keys().size() - 2).isLast());
    CHECK(c.keyControl(c.keys().size() - 1).isLast());
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

  SECTION("Bezier control points can be accessed and modified.")
  {
    auto bezier = BezierInterpolant();
    CHECK(bezier.solve(0.5) == Approx(0.5));

    auto c1 = bezier.control1();
    c1.x = 0.2f;
    c1.y = 1.0f;
    bezier.setControlPoint1(c1);
    CHECK(bezier.solve(0.5) != Approx(0.5));
  }
}
