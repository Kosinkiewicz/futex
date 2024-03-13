/*
 * test.cpp
 *
 *  Created on: Feb 29, 2024
 *      Author: Leszek Kosinkiewicz<kosinkiewicz@gmail.com>
 *
 *  this is under the Apache License Version 2.0.
 *
 */

#include <gtest/gtest.h>

int main(int argc, char * argv[])
{
	::testing::FLAGS_gtest_color = "yes";
	::testing::InitGoogleTest( & argc, argv);
	return RUN_ALL_TESTS();
}
