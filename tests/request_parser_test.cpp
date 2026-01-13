#include <gtest/gtest.h>
#include <request.hpp>
#include <request_parser.hpp>

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

TEST_F(RequestParserTest, HandlesChunkedBody) {
    std::string data = 
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Trailer: Checksum\r\n"
        "\r\n"
        "4\r\nWiki\r\n"
        "5\r\npedia\r\n"
        "0\r\n"
        "Checksum: xxxx\r\n"
        "\r\n";

    auto result = parse_string(data);

    EXPECT_EQ(result, request_parser::good);
    EXPECT_EQ(req.method, "POST");

	std::string expected_str = "Wikipedia";
	std::vector<char> expected_vec(expected_str.begin(), expected_str.end());
    EXPECT_EQ(req.content, expected_vec);
    EXPECT_EQ(req.content.size(), 9);

    bool found_trailer = false;
    for(const auto& h : req.headers) {
        if(h.name == "Checksum") {
            EXPECT_EQ(h.value, "xxxx");
            found_trailer = true;
        }
    }
    EXPECT_TRUE(found_trailer);
}

TEST_F(RequestParserTest, ParsesRequestWithContentLength) {
    std::string head = 
        "POST /submit HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 10\r\n"
        "Content-Type: application/octet-stream\r\n"
        "\r\n";

    std::string body ("ABC\0DEF\0HI", 10); 
    std::string data = head + body;

    auto result = parse_string(data);

    EXPECT_EQ(result, request_parser::good);
    EXPECT_EQ(req.method, "POST");

    bool found_cl = false;
    for(const auto& h : req.headers) {
        if(h.name == "Content-Length") {
            EXPECT_EQ(h.value, "10");
            found_cl = true;
        }
    }
    EXPECT_TRUE(found_cl);

    ASSERT_EQ(req.content.size(), 10);
    
    EXPECT_EQ(req.content[3], '\0');
    EXPECT_EQ(req.content[4], 'D');
    EXPECT_EQ(req.content[7], '\0');
    
    std::vector<char> expected(body.begin(), body.end());
    EXPECT_EQ(req.content, expected);
}
