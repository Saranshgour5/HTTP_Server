#include "request_handler.hpp"
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include <cstddef>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

namespace http {
namespace server {

request_handler::request_handler(const std::string& doc_root)
: doc_root_(doc_root) { }

void request_handler::handle_request(const request& req, reply& rep)
{
	auto url = url_decode(req.uri);
	if (!url.has_value()) 
	{
		rep = reply::stock_reply(reply::BAD_REQUEST);
		return;
	}
	std::string request_path = url.value();
	if (request_path.empty() || request_path.front() != '/')
	{
		rep = reply::stock_reply(reply::BAD_REQUEST);
		return;
	}	
	if (request_path.back() == '/')
	{
		request_path += "index.html";
	}
	std::size_t last_slash_pos = request_path.find_last_of("/");
	std::size_t last_dot_pos = request_path.find_last_of(".");
	std::string extension;
	if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) 
	{
		extension = request_path.substr(last_dot_pos + 1);
	}
	
	std::string full_path = doc_root_ + request_path;
	std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
	if (!is)
	{
		rep = reply::stock_reply(reply::NOT_FOUND);
		return;
	}
	
	rep.status = reply::OK;
	char buf[512];
	while (is.read(buf, sizeof(buf)).gcount() > 0)
		rep.content.append(buf, is.gcount());
	rep.headers.resize(2);
	rep.headers[0].name = "Content-Length";
	rep.headers[0].value = std::to_string(rep.content.size());
	rep.headers[1].name = "Content-Type";
	rep.headers[1].value = mime_types::extension_to_type(extension);
}

std::optional<std::string> url_decode(const std::string& in) 
{
	std::string out{};
	for(std::size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] == '%')
		{
			if (i + 3 <= in.size())
			{
				int value {};
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value)
				{
					out += static_cast<char>(value);
					i += 2;
				}
				else {
					return std::nullopt;
				}
			}
			else { return std::nullopt; }
		}
		else if (in[i] == '+') { out += ' '; }
		else { out += in[i]; }
	}
	return out;
}

} // namespace http
} // namespace server
