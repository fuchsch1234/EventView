/*
 * GTestMain.cpp
 *
 *  Created on: Apr 19, 2020
 *      Author: cf
 */

#include "gtest/gtest.h"

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
