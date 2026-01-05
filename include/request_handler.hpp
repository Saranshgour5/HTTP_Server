#ifndef HTTP_SERVER_REQUEST_HANDLER_HPP
#define HTTP_SERVER_REQUEST_HANDLER_HPP

#include <optional>
#include <string>

namespace http {
namespace server {

struct reply;
struct request;

class request_handler
{
public:
	// INFO: - Constuct with a directory containing the content to be served.
	explicit request_handler(const std::string& doc_root);
	void handle_request(const request& req, reply& rep);
private:
	std::string doc_root_;
	static std::optional<std::string> url_decode(const std::string& input);
};

} //http
} //server
#endif
