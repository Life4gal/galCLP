#include <gtest/gtest.h>

#include <command_line_parser.hpp>

int main(int argc, char **argv) {
	hello();

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
