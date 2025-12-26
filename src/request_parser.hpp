#ifndef HTTP_SERVER_REQUEST_PARSER_HPP
#define HTTP_SERVER_REQUEST_PARSER_HPP

namespace http {
namespace server {

#include <tuple>
struct request;

class request_parser
{
public:
	request_parser();

	void reset();

	enum result_type { good, bad, indeterminate };
	
	template<typename InputIterator>
	std::tuple<result_type, InputIterator> parse(request& req, InputIterator begin, InputIterator end)
	{
		while(begin != end)
		{
			result_type result = consume(req, *begin++);
			if(result == good || result == bad)
				{ return std::make_tuple(result, begin); }
		}
		return std::make_tuple(indeterminate, begin);
	
	}
private:
	result_type consume(request& req, char input);

	static bool is_char(int c);
	static bool is_tspecial(int c);
	static bool is_digit(int c);
	static bool is_ctl(int c);

	enum state
	{
		method_start,
		method,
		url,
		http_version_h,
		http_version_t_1,
		http_version_t_2,
		http_version_t_3,
		http_version_p,
		http_version_slash,
		http_version_major_start,
		http_version_major,
		http_version_major_dot,
		http_version_minor,
		expecting_newline_1,
		header_line_start,
		header_lws,
		header_name,
		space_before_header_value,
		header_value,
		expecting_newline_2,
		expecting_newline_3
	} state_;
};

}
}
#endif
