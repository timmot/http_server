#include "log.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class Method {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH
};

struct HttpRequest {
    Method method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;

    std::string method_name()
    {
        switch (method) {
        case Method::GET:
            return "GET";
        case Method::HEAD:
            return "HEAD";
        case Method::POST:
            return "POST";
        case Method::PUT:
            return "PUT";
        case Method::DELETE:
            return "DELETE";
        case Method::CONNECT:
            return "CONNECT";
        case Method::OPTIONS:
            return "OPTIONS";
        case Method::TRACE:
            return "TRACE";
        case Method::PATCH:
            return "PATCH";
        default:
            return "";
        }
    }

    inline void debug()
    {
        logln("Method: {}", method_name());
        logln("Path: {}", path);

        logln("Headers:");
        for (auto pair : headers) {
            logln("{}:{}", pair.first, pair.second);
        }
    }
};

std::vector<std::string> split(std::string const& text, char separator)
{
    std::vector<std::string> output;

    for (int i = 0; i < text.size();) {
        auto j = text.find_first_of(separator, i);
        if (j == std::string::npos) {
            j = text.size() - 1;
            output.push_back(text.substr(i, j - i));
            break;
        }
        output.push_back(text.substr(i, j - i));
        i = j + 1;
    }

    return output;
}

std::vector<std::string_view> split_view(std::string_view text, char separator)
{
    std::vector<std::string_view> output;

    for (int i = 0; i < text.size();) {
        auto j = text.find_first_of(separator, i);
        if (j == std::string::npos) {
            j = text.size() - 1;
            auto view = text.substr(i, j - i);
            output.push_back(view);
            break;
        }
        auto view = text.substr(i, j - i);
        output.push_back(view);
        i = j + 1;
    }

    return output;
}

/* RFC9112 - 3.
     HTTP-message   = start-line
                      *( header-field CRLF )
                      CRLF
                      [ message-body ]
*/

// The normal procedure for parsing an HTTP message is to read the start-line into a structure, read each header field line into a hash table by field name until the empty line, and then use the parsed data to determine if a message body is expected. If a message body has been indicated, then it is read as a stream until an amount of octets equal to the message body length is read or the connection is closed.

std::optional<HttpRequest> parse_html(std::string const& request)
{
    // TODO: Exchange this for socket code
    // TODO: Clear all excess \r (CR) as in spec.
    auto lines = split_view(request, '\n');
    int read_lines = 0;
    auto get_line = [&lines, &read_lines]() {
        if (read_lines >= lines.size())
            return std::optional<std::string_view> {};
        return std::optional { lines[read_lines++] };
    };

    auto request_line = split_view(*get_line(), ' ');
    if (request_line.size() != 3) {
        logln("Wrong amount of args in request line");
        exit(1);
    }

    std::unordered_map<std::string, std::string> headers;
    for (;;) {
        auto header_line = get_line();

        if (!header_line.has_value() || header_line == "\r")
            break;

        // FIXME: Doesn't handle multi-occurring headers
        // FIXME: Doesn't handle list value headers
        auto header_parts = split_view(*header_line, ':');

        if (header_parts.size() < 2) {
            logln("Header line has no colon");
            continue;
        }

        auto header_colon = header_line->find_first_of(':') + 1;
        auto header_value_length = header_line->size() - 1 - header_colon;
        headers.emplace(header_parts[0], header_line->substr(header_colon, header_value_length));
    }

    // TODO: Read body?

    HttpRequest http_request = {};
    auto method = request_line[0];
    if (method == "GET")
        http_request.method = Method::GET;
    else if (method == "HEAD")
        http_request.method = Method::HEAD;
    else if (method == "POST")
        http_request.method = Method::POST;
    else if (method == "PUT")
        http_request.method = Method::PUT;
    else if (method == "DELETE")
        http_request.method = Method::DELETE;
    else if (method == "CONNECT")
        http_request.method = Method::CONNECT;
    else if (method == "OPTIONS")
        http_request.method = Method::OPTIONS;
    else if (method == "TRACE")
        http_request.method = Method::TRACE;
    else if (method == "PATCH")
        http_request.method = Method::PATCH;
    else {
        logln("Invalid method in request line");
        exit(1);
    }

    http_request.path = request_line[1];
    http_request.headers = std::move(headers);

    return http_request;
}

int main()
{
    // https://www.rfc-editor.org/rfc/rfc9110
    // https://www.rfc-editor.org/rfc/rfc9112
    // https://rfcs.io/http

    std::string request = "GET /index.html HTTP/1.1\r\n"
                          "Host: localhost:8080\r\n"
                          "User-Agent: curl/7.84.0\r\n"
                          "Accept: */*\r\n\r\n";

    auto http_request = parse_html(request);

    if (http_request.has_value())
        http_request->debug();

    return 0;

    // Re: Managing partial messages in a Keep-Alive
    // If a valid Content-Length header field is present without Transfer-Encoding, its decimal value defines the expected message body length in octets. If the sender closes the connection or the recipient times out before the indicated number of octets are received, the recipient MUST consider the message to be incomplete and close the connection.

    // https://www.rfc-editor.org/rfc/rfc9112#name-message-body-length

    // https://www.rfc-editor.org/rfc/rfc9112#name-persistence
}