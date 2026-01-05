#ifndef  HTTP_SERVER_REPLY_HPP
#define HTTP_SERVER_REPLY_HPP

#include <boost/asio/buffer.hpp>
#include <string>
#include <boost/asio.hpp>
#include <vector>
#include "header.hpp"

namespace http {
namespace server {

struct reply
{
	enum status_type
	{
		OK = 200,
		CREATED = 201,
		ACCEPTED = 202,
		NO_CONTENT = 204,
		MULTIPLE_CHOICES = 300,
		MOVED_PERMANENTLY = 301,
		MOVED_TEMPORARILY = 302,
		BAD_REQUEST = 400,
		FORBIDDEN = 403,
		NOT_FOUND = 404,
		INTERNAL_SERVER_ERROR = 500,
		NOT_IMPLEMENTED = 501,
		SERVICE_UNAVAILABLE = 503
	} status;

	std::vector<header> headers;
	std::string content;
	std::vector<boost::asio::const_buffer> to_buffers();
	static reply stock_reply(status_type status);
};

}
}
#endif

