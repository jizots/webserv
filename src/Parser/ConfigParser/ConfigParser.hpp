#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "Utils/Types.hpp"

#define SPECIAL_CHARS "\'\"|()^$+?=\\"
#define DELIMITER_CHARS "{};"

namespace webserv
{
/*************
 * Constants syntax
 * *************/

enum eContext
{
    MAIN,
    HTTP,
    SERVER,
    LOCATION,
    CONTEXT_ENUM_SIZE,
};

const std::string CONFIG_CONTEXTS[CONTEXT_ENUM_SIZE] =
{
    "main",
    "http",
    "server",
    "location"
};

enum eSimpleDirective
{
    ERROR_LOG,
    LISTEN,
    SERVER_NAME,
    UPLOAD_PATH,
    ROOT,
    ERROR_PAGE,
    AUTOINDEX,
    CLIENT_MAX_BODY_SIZE,
    INDEX,
    REDIRECT,
    ACCEPTED_CGI_EXTENSION,
    ACCEPTED_METHODS,
    ALIAS,
    DIRECTIVE_ENUM_SIZE,
};

const std::string SIMPLE_DIRECTIVES[DIRECTIVE_ENUM_SIZE] = 
{
    "error_log",
    "listen",
    "server_name",
    "upload_path",
    "root",
    "error_page",
    "autoindex",
    "client_max_body_size",
    "index",
    "redirect",
    "accepted_cgi_extension",
    "accepted_methods",
    "alias",
};

/*************
 * Lexer struct
 * *************/
struct Token
{
    std::string value;
    size_t line;
};

struct SimpleDirective
{
    std::string name;
    std::vector<std::string> parameters;
};

struct BlockDirective
{
    std::string nameContext;
    std::string parameter;
    std::vector<SimpleDirective> directives;
    std::vector<BlockDirective> blockDirectives;
};

struct MainDirective
{
    std::vector<SimpleDirective> directives;
    std::vector<BlockDirective> blocks;
};

/*************
 * Parser struct
 * *************/
// helper struct for multiple parameters
struct MultipleParameter
{
    static std::vector<std::string> listen;
    static std::vector<std::string> serverName;
    static std::vector<std::string> errorPage;
    static std::vector<std::string> cgiPath;
};

/*
 * For location directive items.
 */
struct LocationDirective
{
    std::string location;
    std::string root;
    std::string redirect;
    std::map<int, std::string> error_page;
    bool autoindex;
    bool isSetAutoindex;
    uint64 client_max_body_size;
    bool isSetMaxBody;
    std::string index;
    std::vector<std::string> accepted_methods;
    std::map<std::string, std::string> accepted_cgi_extension;
    std::string alias;

    std::string translateURI(const std::string& uri) const;
    bool isMatching(const std::string& uri) const;
};

/*
 * Datas pass to the each servers.
 */
struct ServerConfig
{
    int serverID;
    std::string error_log;
    std::vector<uint16> listens;
    std::vector<std::string> server_names;
    std::string upload_path;
    std::vector<LocationDirective> locations;

    const LocationDirective& bestLocation(const std::string& uri);

private:
    std::string m_lastUri;
    std::vector<LocationDirective>::size_type m_lastLocationIdx;
};

/*************
 * Functions
 * *************/
std::vector<ServerConfig> parseServerConfig(const int ac, const char** av);
std::ostream& operator<<(std::ostream& os, const std::vector<ServerConfig>& servers);

/*************
 * Exceptions
 * *************/
class ConfigException : public std::exception
{
public:
    ConfigException(const std::string& level, const size_t& lineNo, const std::string& description, const std::string& token);
    inline ~ConfigException(void) throw(){};
    inline virtual const char* what() const throw() { return (m_msg.c_str()); };

private:
    std::string m_msg;
};

/*************
 * Template
 * *************/
template<typename T>
void printVector(const std::vector<T> vec)
{
    for (size_t i = 0; i < vec.size(); ++i)
        std::cout << vec[i] << std::endl;
};

} // namespace webserv

#endif
