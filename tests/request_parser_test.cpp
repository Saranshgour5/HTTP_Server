#include <gtest/gtest.h>
#include "../src/request.hpp"
#include "../src/request_parser.hpp"

using namespace http::server;

class RequestParserTest: public ::testing::Test {
protected:
	request_parser parser;
	request req;

	request_parser::result_type parse_string(const std::string& input) {
		request_parser::result_type result;
		std::string::const_iterator it;

		std::tie(result, it) = parser.parse(req, input.begin(), input.end());
		return result;
	}
};

TEST_F(RequestParserTest, ParsesValidGetRequest) {
    std::string data = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n"
        "\r\n";

    auto result = parse_string(data);

    EXPECT_EQ(result, request_parser::good);
    EXPECT_EQ(req.method, "GET");
    ASSERT_EQ(req.headers.size(), 2);
    EXPECT_EQ(req.headers[0].name, "Host");
}

TEST_F(RequestParserTest, FailsOnInvalidMethod) {
    std::string data = "GET\001 / HTTP/1.1\r\n";
    
    auto result = parse_string(data);
    
    EXPECT_EQ(result, request_parser::bad);
}

TEST_F(RequestParserTest, HandlesIncrementalData) {
    std::string chunk1 = "GET /index.ht";
    std::string chunk2 = "ml HTTP/1.1\r\n\r\n";

    auto res1 = parse_string(chunk1);
    EXPECT_EQ(res1, request_parser::indeterminate);

    auto res2 = parse_string(chunk2);
    EXPECT_EQ(res2, request_parser::good);
}
