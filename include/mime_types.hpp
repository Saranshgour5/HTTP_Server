#ifndef HTTP_SERVER_MIME_TYPES_HPP
#define HTTP_SERVER_MIME_TYPES_HPP

#include <string>

namespace http {
namespace server {
namespace mime_types {

std::string extension_to_type(const std::string& extension);

} //namespace mime_types

} // namespace server
} // namepsacer http
#endif
