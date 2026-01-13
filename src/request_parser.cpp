#include "header.hpp"
#include "request.hpp"
#include "request_parser.hpp"

#include <boost/algorithm/string.hpp>
#include <cctype>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <strings.h>
#include <iostream>

namespace http {
namespace server {

request_parser::request_parser()
: state_(method_start)
{
}

void request_parser::reset()
{
	state_ = method_start;
}
request_parser::result_type request_parser::consume(request& req, char input)
{
	switch (state_)
	{
		case method_start:
		{

			if (!is_char(input) || is_ctl(input) || is_tspecial(input))
			{
				return bad;
			}
			else
			{
				state_ = method;
				req.method.push_back(input);
				return indeterminate;
			}
		}
		case method:
		{

			if (input == ' ')
			{
				state_ = url;
				return indeterminate;
			}
			else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
			{
				return bad; }
			else
			{
				req.method.push_back(input);
				return indeterminate;
			}
		}
		case url:
		{

			if (input == ' ')
			{
				state_ = http_version_h;
				return indeterminate;
			}
			else if (is_ctl(input))
			{
				return bad;
			}
			else
			{
				req.uri.push_back(input);
				return indeterminate;
			}
		}
		case http_version_h:
		{

			if (input == 'H')
			{
				state_ = http_version_t_1;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case http_version_t_1:
		{

			if (input == 'T')
			{
				state_ = http_version_t_2;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case http_version_t_2:
		{

			if (input == 'T')
			{
				state_ = http_version_p;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case http_version_p:
		{
			if (input == 'P')
			{
				state_ = http_version_slash;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case http_version_slash:
		{
			if (input == '/')
			{
				req.http_version_major = 0;
				req.http_version_minor = 0;
				state_ = http_version_major_start;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case http_version_major_start:
		{

			if (is_digit(input))
			{
				req.http_version_major = req.http_version_major * 10 + input - '0';
				state_ = http_version_major;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case http_version_major:
		{
			if (input == '.')
			{
				state_ = http_version_major_dot;
				return indeterminate;
			}
			else if (is_digit(input))
			{
				req.http_version_major = req.http_version_major * 10 + input - '0';
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case http_version_major_dot:
		{
			if (is_digit(input))
			{
				req.http_version_minor = req.http_version_minor * 10 + input - '0';
				state_ = http_version_minor;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case http_version_minor:
		{

			if (input == '\r')
			{
				state_ = expecting_newline_1;
				return indeterminate;
			}
			else if (is_digit(input))
			{
				req.http_version_minor = req.http_version_minor * 10 + input - '0';
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case expecting_newline_1:
		{
			if (input == '\n')
			{
				state_ = header_line_start;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case header_line_start:
		{

			if (input == '\r')
			{
				state_ = expecting_newline_3;
				return indeterminate;
			}
			else if (!req.headers.empty() && (input == ' ' || input == '\t'))
			{
				state_ = header_lws;
				return indeterminate;
			}
			else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
			{
				return bad;
			}
			else
			{
				req.headers.push_back(header());
				req.headers.back().name.push_back(input);
				state_ = header_name;
				return indeterminate;
			}
		}
		case header_lws:
		{
			if (input == '\r')
			{
				state_ = expecting_newline_2;
				return indeterminate;
			}
			else if (input == ' ' || input == '\t')
			{
				return indeterminate;
			}
			else if (is_ctl(input))
			{
				return bad;
			}
			else
			{
				state_ = header_value;
				req.headers.back().value.push_back(input);
				return indeterminate;
			}
		}
		case header_name:
		{
			if (input == ':')
			{
				state_ = space_before_header_value;
				return indeterminate;
			}
			else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
			{
				return bad;
			}
			else
			{
				req.headers.back().name.push_back(input);
				return indeterminate;
			}
		}
		case space_before_header_value:
		{
			if (input == ' ')
			{
				state_ = header_value;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case header_value:
		{
			if (input == '\r')
			{
				const auto& header = req.headers.back();
				if (boost::iequals(header.name, "Content-Length")) {
					if(std::all_of(header.value.begin(), header.value.end(), is_digit)) 
					{
						content_length_ = atoi(header.value.c_str());
						//TODO: Check against overflow
						req.content.reserve(content_length_)	;
					}
					else { return bad; }
				}
				else if (boost::iequals(header.name, "Transfer-Encoding") && boost::iequals(header.value, "Chunked"))
				{
					chunked_ = true;
				}

				state_ = expecting_newline_2;
				return indeterminate;
			}
			else if (is_ctl(input))
			{
				return bad;
			}
			else
			{
				req.headers.back().value.push_back(input);
				return indeterminate;
			}
		}
		case expecting_newline_2:
		{
			if (input == '\n')
			{
				state_ = header_line_start;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		case expecting_newline_3:
		{
			if (chunked_)
			{
				state_ = chunk_size_start;
				return indeterminate;
			}
			else if (0 == content_length_)
			{
				if (input == '\n') return good;
				else return bad; //INFO: Not reading till eof if content-length is missing and chunked is also missing
			}
			else 
			{
				//Check if needed to read until eof
				state_ = body_identity;
				return indeterminate;
			}
		}
		case body_identity:
		{
			--content_length_;
			req.content.push_back(input);
			if (content_length_ == 0) { return good; }
			return indeterminate;
		}
		case chunk_size_start: 
		{
			int unhex_val = unhex(input);
			if (unhex_val != -1)
			{
				content_length_ = unhex_val;
				state_ = chunk_size;
			}
			else 
			{
				return bad;
			}
			return indeterminate;
		}
		case chunk_size:
		{
			uint64_t len {};
			if (input == '\r')
			{
				state_ = chunk_size_almost_done;
				return indeterminate;
			}

			int unhex_val = unhex(input);
			if (unhex_val == -1) {
				if(input == ';' || input == ' ')
				{
					state_ = chunk_parameters;
					return indeterminate;
				}
				else {
					return bad;
				}
			}
			// if (unhex_val != -1)
			// {
			// 	content_length_ = unhex_val;
			// 	state_ = chunk_size;
			// }
			// else 
			// {
			// 	return bad;
			// }
			//TODO: Add overflow test
			len = content_length_;
			len *= 16;
			len += unhex_val;
			content_length_ = len;
			return indeterminate;
		}
		case chunk_parameters:
		{
			//INFO: Ignore this shit
			//WARN: Check for overflow
			if(input == '\r') {
				state_ = chunk_size_almost_done;
			}
			return indeterminate;
		}
		case chunk_size_almost_done:
		{
			if (input == '\n')
			{
				req.content.reserve(req.content.size() + content_length_);
				if (0 == content_length_) {
					//INFO: Storing trailing headers with the headers
					chunked_ = false;
					state_ = header_line_start; 
				}
				else { state_ = chunk_data; }
				return indeterminate;
			}
			else { return bad; }
		}
		case chunk_data:
		{
			req.content.push_back(input);
			if (--content_length_ == 0)
			{
				state_ = chunk_data_almost_done;
			}
			return indeterminate;
		}
		case chunk_data_almost_done:
		{
			if (input == '\r')
			{
				state_ = chunk_data_done;
				return indeterminate;
			}	
			else 
			{
				return bad;
			}
		}
		case chunk_data_done:
		{
			if (input == '\n')
			{
				state_ = chunk_size_start;
				return indeterminate;
			}
			else
			{
				return bad;
			}
		}
		default:
			return bad;
	}
}

bool request_parser::is_char(int c)
{
	return c >= 0 && c <= 127;
}

bool request_parser::is_ctl(int c)
{
	return (c >= 0 && c <= 31) || (c == 127);
}

bool request_parser::is_tspecial(int c)
{
	switch (c)
	{
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
			return true;
		default:
			return false;
	}
}

bool request_parser::is_digit(int c)
{
	return c >= '0' && c <= '9';
}

int request_parser::unhex(char c)
{
	static std::vector<int> s_hex_digit(256, -1);
	for (std::size_t i = '0'; i <= '9'; ++i) { s_hex_digit[i] = i - '0'; }
	for (std::size_t i = 'A'; i <= 'F'; ++i) { s_hex_digit[i] = 10 + (i - 'A'); }
	for (std::size_t i = 'a'; i <= 'f'; ++i) { s_hex_digit[i] = 10 + (i - 'a'); }
	return s_hex_digit[static_cast<unsigned char>(c)];
}

} //namespace server
} //namespace http
