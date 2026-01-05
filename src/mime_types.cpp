#include "mime_types.hpp"
#include <string>
#include <unordered_map>

namespace http {
namespace server {
namespace mime_types {

const std::unordered_map<std::string, std::string> mapping  {
	{ "gif", "image/gif" },
	{ "htm", "text/html" },
	{ "html", "text/html" },
	{ "jpg", "image/jpeg" },
	{ "png", "image/png" }
};

std::string extension_to_type(const std::string &extension)
{
	if(mapping.contains(extension)) return mapping.at(extension);
	return "text/html";
}
} // namespace mime_types

} // namespace server
} // namespace http
