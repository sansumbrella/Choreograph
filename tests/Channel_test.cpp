//
//  Channel_testTest.cpp
//
//  Created by Soso Limited on 1/11/16.
//
//

#include "catch.hpp"

TEST_CASE("Channel_testTest")
{
	auto a = 0;
	auto b = 1;

	SECTION("Write a failing test first to confirm your new test is run.")
	{
		REQUIRE(a == b);
	}
}
