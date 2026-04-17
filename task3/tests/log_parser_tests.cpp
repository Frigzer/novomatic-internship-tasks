#include <gtest/gtest.h>

#include "log_parser.hpp"

namespace task3 {

TEST(LogParserTests, ParsesValidLine) {
    const std::string line =
        "[2023-10-25T10:05:12] [ERROR] [Database] Connection timeout";

    auto parsed = LogParser::parseLine(line);

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->level, LogLevel::Error);
    EXPECT_EQ(parsed->source, "Database");
    EXPECT_EQ(parsed->message, "Connection timeout");
    EXPECT_EQ(parsed->raw_line, line);
}

TEST(LogParserTests, ReturnsNulloptForInvalidFormat) {
    const std::string line = "invalid log line";

    auto parsed = LogParser::parseLine(line);

    EXPECT_FALSE(parsed.has_value());
}

}  // namespace task3