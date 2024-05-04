#include "ConfigParser.hpp"
#include "Utils/Functions.hpp"
#include "Utils/Macros.hpp"
#include "Utils/Logger.hpp"
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
#include <algorithm>
#include <unistd.h>
#include <iostream> /*debug*/

namespace webserv
{

/**************
 * Debug utils
************/
std::ostream&				operator<<(std::ostream& os, const std::vector<ServerConfig>& servers)
{
	for (size_t j = 0; j < servers.size(); ++j)
	{
		os << "------ ServerConfig#" << servers[j].serverID << std::endl;
		os << "error_log: " << servers[j].error_log << std::endl;
		os << "listens: "; printVector<uint16>(servers[j].listens);
		os << "server_names: "; printVector<std::string>(servers[j].server_names);
		os << "upload_path: " << servers[j].upload_path << std::endl;
		for (size_t i = 0; i < servers[j].locations.size(); ++i)
		{
			os << "------ location: ";
			os << servers[j].locations[i].location << std::endl;
			os << "root: " << servers[j].locations[i].root << std::endl;
			os << "redirect: " << servers[j].locations[i].redirect << std::endl;
			os << "error_page: ";
			for (std::map<int, std::string>::const_iterator it = servers[j].locations[i].error_page.begin(); it != servers[j].locations[i].error_page.end(); ++it)
				os << it->first << " => " << it->second << ", ";
			os << std::endl;
			os << "autoindex: " << servers[j].locations[i].autoindex << std::endl;
			os << "client_max_body_size: " << servers[j].locations[i].client_max_body_size << std::endl;
			os << "index: " << servers[j].locations[i].index << std::endl;
			os << "accepted_methods: "; printVector<std::string>(servers[j].locations[i].accepted_methods);
			os << "accepted_cgi_extension: ";
			for (std::map<std::string, std::string>::const_iterator it = servers[j].locations[i].accepted_cgi_extension.begin(); it != servers[j].locations[i].accepted_cgi_extension.end(); ++it)
				os << it->first << " => " << it->second << ", ";
			os << std::endl;
			os << "alias: " << servers[j].locations[i].alias << std::endl;
		}
	}
	return (os);
};

/**************
 * readFileToString
************/
static std::string	readFileToString(const char *filePath)
{
	std::ifstream	ifs(filePath);
	std::string		fileContents;

    try
    {
	    isFileAccessible(filePath);
    }
    catch (std::exception& e)
    {
        throw(ConfigException("error", 0, e.what(), ""));
    }
	fileContents.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	ifs.close();
	return (fileContents);
};

/**************
 * Tokenize in Lexing
************/
static bool	ignoreComment(const char& c, bool& isComment, size_t& line)
{
	if (c == '#')
		isComment = true;
	if (isComment)
	{
		if (c == '\n' || c == '\0')
		{
			isComment = false;
			++line;
		}
		return (true);
	}
	return (false);
};

static void	addTokenToVector(std::vector<Token>& tokens, std::string& token, size_t line)
{
	Token	newToken;

	newToken.value = token;
	newToken.line = line;
	tokens.push_back(newToken);
	token.clear();
};

static std::vector<Token>	tokenize(const std::string& str)
{
	char				c;
	std::string			token;
	size_t				line = 1;
	std::vector<Token> 	tokens;
	bool				isComment = false;

	for (size_t i = 0; i < str.length(); ++i)
	{
		c = str[i];
		if (ignoreComment(c, isComment, line))
			continue;
		if (IS_IN(SPECIAL_CHARS, c))
			throw (ConfigException("error", line, "Can't use", std::string(1, c)));
		if (c == '\n')
			++line;
		if (std::isspace(c) || IS_IN(DELIMITER_CHARS, c))
		{
			if (!token.empty())
				addTokenToVector(tokens, token, line);
			if (IS_IN(DELIMITER_CHARS, c))
			{
				token += c;
				addTokenToVector(tokens, token, line);
			}
		}
		else
			token += c;
	}
	if (!token.empty())
		addTokenToVector(tokens, token, line);
	return (tokens);
}

static int	searchContextName(const std::string& name)
{
	for (int i = 0; i < CONTEXT_ENUM_SIZE; ++i)
	{
		if (name == CONFIG_CONTEXTS[i])
			return (i);
	}
	return (-1);
}

static int	searchDirectiveName(const std::string& name)
{
	for (int i = 0; i < DIRECTIVE_ENUM_SIZE; ++i)
	{
		if (name == SIMPLE_DIRECTIVES[i])
			return (i);
	}
	return (-1);
}

/**************
 * Syntax check in Lexing
************/
static bool	isValidDirective(const Token& name, const std::string& parentContext)
{
	int	nameID = searchDirectiveName(name.value);
	int	parentID = searchContextName(parentContext);

	if (nameID == -1 || parentID == -1)
		return (false);
	if (parentID == MAIN 
			&& nameID == ERROR_LOG)
		return (true);
	else if (parentID == HTTP)
		return (false);
	else if (parentID == SERVER
			&& (LISTEN <= nameID && nameID <= ACCEPTED_CGI_EXTENSION))
		return (true);
	else if (parentID == LOCATION
			&& (ROOT <= nameID && nameID <= ALIAS))
		return (true);
	return (false);
};

static bool	isValidBlock(const Token& block, const std::string& parentContext)
{
	int	nameID = searchContextName(block.value);
	int	parentID = searchContextName(parentContext);

	if (nameID == -1 || parentID == -1)
		return (false);
	if (parentID == MAIN
			&& nameID == HTTP)
		return (true);
	else if (parentID == HTTP
			&& nameID == SERVER)
		return (true);
	else if (parentID == SERVER
			&& nameID == LOCATION)
		return (true);
	return (false);
};

static void	incrementIndex(const std::vector<Token>& tokens, int& index)
{
	++index;
	if (static_cast<uint64>(index) == tokens.size())
	{
		--index;
		throw (ConfigException("error" , tokens[index].line
			, "Unexpected line ending with", tokens[index].value));
	}
}

static bool	isValidErrorPage(const std::vector<Token>& tokens,
	int& index)
{
	std::string tmp;
	int 		errorCode;
	bool		hasNumber = false;

	while (!(IS_IN(DELIMITER_CHARS, tokens[index + 1].value[0]))
		&& tokens[index].value != ";"
		&& (index < 0 || static_cast<uint64>(index) < tokens.size() - 1))
	{
		tmp = tokens[index].value;
		if (is<int>(tmp))
			errorCode = to<int>(tmp);
		else
			throw (ConfigException("error", tokens[index].line, "invalid error code", tmp));
		if (errorCode < 200 || 599 < errorCode)
			throw (ConfigException("error", tokens[index].line, "invalid error code", tmp));
		tmp.clear();
		incrementIndex(tokens, index);
		hasNumber = true;
	}
	if (IS_IN(DELIMITER_CHARS, tokens[index + 1].value[0]) && hasNumber)
		return (true);
	if (!hasNumber)
		throw (ConfigException("error", tokens[index].line, "invalid number of argumentes 'error_page'", ""));
	return (false);
};

static bool	isValidAcceptedCgiExtension(const std::vector<Token>& tokens,
	int& index)
{
	if (tokens[index].value[0] != '.' || tokens[index].value.size() < 2)
		return (false);
	if (IS_IN(DELIMITER_CHARS, tokens[index + 1].value[0]))
		return (true);
	incrementIndex(tokens, index);
	if (access(tokens[index].value.c_str(), X_OK) == 0)
		return (true);
	throw (ConfigException("error",  tokens[index].line
		, "access(): " + std::string(std::strerror(errno)), to_string(tokens[index].value)));
};

static bool	isValidAcceptedMethods(const std::vector<Token>& tokens,
	int& index)
{
	std::string	tmp;

	while (tokens[index].value != ";"
		&& (index < 0 || static_cast<uint64>(index) < tokens.size() - 1))
	{
		tmp = tokens[index].value;
		if (tmp == "GET" || tmp == "POST" || tmp == "DELETE" || tmp == "PUT" || tmp == "HEAD")
		{
			tmp.clear();
			if (tokens[index + 1].value == ";")
				return (true);
			incrementIndex(tokens, index);
		}
		else
			throw (ConfigException("error", tokens[index].line, "Invalid method", tmp));
	}
	return (false);
};

static bool	isValidParam(const std::vector<Token>& tokens,
	int& index)
{
	std::string	param = tokens[index].value;
	int		nameID = searchDirectiveName(tokens[index - 1].value);

	if (IS_IN(DELIMITER_CHARS, param[0]))
		return (false);
	switch(nameID)
	{
		case ERROR_LOG:
			return (true);
		case LISTEN:
			if (param == "xxx") //for test
				return (true);
			if (is<uint16>(param))
				return (true);
			return (false);
		case SERVER_NAME:
			return (true);
		case UPLOAD_PATH:
			if (param[param.size() - 1] == '/')
				return (true);
			throw (ConfigException("error", tokens[index].line, "upload_path must end with slash(/)", param));
		case ROOT:
			return (true);
		case ERROR_PAGE:
			if (!isValidErrorPage(tokens, index))
				return (false);
			return (true);
		case AUTOINDEX:
			if (param != "on" && param != "off")
				return (false);
			return (true);
		case CLIENT_MAX_BODY_SIZE:
			if (is<uint64>(param))
				return (true);
			return (false);
		case INDEX:
			if (param[0] == '/')
				throw (ConfigException("error", tokens[index].line, "index must not start with slash(/)", param));
			return (true);
		case REDIRECT:
			return (true);
		case ACCEPTED_CGI_EXTENSION:
			if (isValidAcceptedCgiExtension(tokens, index))
				return (true);
			return (false);
		case ACCEPTED_METHODS:
			if (!isValidAcceptedMethods(tokens, index))
				return (false);
			return (true);
		case ALIAS:
			return (true);
		case -1:
			return (false);
		default:
			return (true);
	}
};

static void	resetParentContext(std::string& parentContext)
{
	if (parentContext == CONFIG_CONTEXTS[LOCATION])
		parentContext = CONFIG_CONTEXTS[SERVER];
	else if (parentContext == CONFIG_CONTEXTS[SERVER])
		parentContext = CONFIG_CONTEXTS[HTTP];
	else if (parentContext == CONFIG_CONTEXTS[HTTP])
		parentContext = CONFIG_CONTEXTS[MAIN];	
}

static BlockDirective*	getLastHttpBlock(MainDirective& mainDir)
{
	if (mainDir.blocks.size() == 0)
		return (NULL);
	return (&mainDir.blocks[mainDir.blocks.size() - 1]);
};

static BlockDirective*	getLastServerBlock(MainDirective& mainDir)
{
	if (mainDir.blocks.size() == 0)
		return (NULL);
	return (&(getLastHttpBlock(mainDir)
		->blockDirectives[getLastHttpBlock(mainDir)->blockDirectives.size() - 1]));
};

static BlockDirective*	getLastLocationBlock(MainDirective& mainDir)
{
	if (mainDir.blocks.size() == 0)
		return (NULL);
	return (&(getLastServerBlock(mainDir)
		->blockDirectives[getLastServerBlock(mainDir)->blockDirectives.size() - 1]));
};

static void	addDirectiveToStruct(MainDirective& mainDir,
	const std::vector<Token>& tokens, int nameIndex, int semicoronIndex,
	const std::string& parentContext)
{
	SimpleDirective	simpleDir;

	simpleDir.name = tokens[nameIndex].value;
	for (int i = nameIndex + 1; i < semicoronIndex; ++i)
		simpleDir.parameters.push_back(tokens[i].value);
	if (searchContextName(parentContext) == MAIN)
		mainDir.directives.push_back(simpleDir);
	else if (searchContextName(parentContext) == HTTP)
		getLastHttpBlock(mainDir)->directives.push_back(simpleDir);
	else if (searchContextName(parentContext) == SERVER)
		getLastServerBlock(mainDir)->directives.push_back(simpleDir);
	else if (searchContextName(parentContext) == LOCATION)
		getLastLocationBlock(mainDir)->directives.push_back(simpleDir);
};

static void	addBlockToStruct(const std::string& parentContext,
	MainDirective& mainDir, const Token& name, const Token& locationName)
{
	int	parentID = searchContextName(parentContext);
	BlockDirective	blockDir;

	if (parentID == LOCATION)
	{
		blockDir.nameContext = locationName.value;
		blockDir.parameter = name.value;
		if (IS_IN(DELIMITER_CHARS, name.value[0]))
			throw (ConfigException("error", name.line, "invalid location name", name.value));
	}
	else
	{
		blockDir.nameContext = name.value;
		blockDir.parameter = "";
	}
	if (parentID == HTTP)
		mainDir.blocks.push_back(blockDir);
	else if (parentID == SERVER)
		getLastHttpBlock(mainDir)->blockDirectives.push_back(blockDir);
	else if (parentID == LOCATION)
		getLastServerBlock(mainDir)->blockDirectives.push_back(blockDir);
};

static bool	isSimpleDirective(const std::vector<Token>& tokens,
	MainDirective& mainDir, int& index, const std::string& parentContext)
{
	int			nameIndex;

	if (isValidDirective(tokens[index], parentContext))
	{
		nameIndex = index;
		incrementIndex(tokens, index);
		if (isValidParam(tokens, index))
		{
			incrementIndex(tokens, index);
			if (tokens[index].value == ";")
			{
				addDirectiveToStruct(mainDir, tokens, nameIndex, index, parentContext);
				++index;
				return (true);
			}
		}
		throw (ConfigException("error", tokens[index].line, "Unexpected token", tokens[index].value));
	}
	return (false);
};

static bool	isBlockDirective(const std::vector<Token>& tokens,
	MainDirective& mainDir, int& index, std::string& parentContext)
{
	if (isValidBlock(tokens[index], parentContext))
	{
		parentContext = tokens[index].value;
		if (tokens[index].value == CONFIG_CONTEXTS[LOCATION])
			incrementIndex(tokens, index);
		incrementIndex(tokens, index);
		if (tokens[index].value == "{")
		{
			addBlockToStruct(parentContext, mainDir, tokens[index - 1], tokens[index - 2]);
			++index;
			return (true);
		}
		throw (ConfigException("error", tokens[index].line, "Unexpected token", tokens[index].value));
	}
	return (false);
};

static bool	isEndOfBlock(const std::vector<Token>& tokens,
	int& index, std::string& parentContext, int hasItem)
{
	if (searchContextName(parentContext) != MAIN
		&& 0 < hasItem
		&& tokens[index].value == "}")
	{
		++index;
		resetParentContext(parentContext);
		return (true);
	}
	throw (ConfigException("error", tokens[index].line, "Unexpected token", tokens[index].value));
	return (false);
}

static bool	makeTmpStruct(const std::vector<Token>& tokens,
	MainDirective& mainDir, std::string parentContext, int index, int hasItem)
{
	if (isSimpleDirective(tokens, mainDir, index, parentContext))
		++hasItem;
	else if (isBlockDirective(tokens, mainDir, index, parentContext))
		hasItem = 0;
	else if (isEndOfBlock(tokens, index, parentContext, hasItem))
		++hasItem;
	if (static_cast<uint64>(index) == tokens.size())
	{
		if (hasItem == 0 || searchContextName(parentContext) != MAIN)
			throw (ConfigException("error", 0, "Block is not closed or empty", ""));
		return (true);
	}
	return (makeTmpStruct(tokens, mainDir, parentContext, index, hasItem));
};

/**************
 * Duplication check
************/
std::vector<std::string>	MultipleParameter::listen;
std::vector<std::string>	MultipleParameter::serverName;
std::vector<std::string>	MultipleParameter::errorPage;
std::vector<std::string>	MultipleParameter::cgiPath;

static bool isGradualVectorDuplicate(std::vector<std::string>& strage, const std::vector<std::string>& add)
{
	for (size_t i = 0; i < add.size(); ++i)
		strage.push_back(add[i]);
	if (hasDuplicate(strage))
		return (true);
	return (false);
};

static void clearMultipleParameter()
{
	MultipleParameter::listen.clear();
	MultipleParameter::serverName.clear();
	MultipleParameter::errorPage.clear();
};

static bool hasUniqMultiParameter(const int& nameId, const std::vector<std::string>& params)
{
	MultipleParameter	multiple;

	if (nameId == LISTEN)
	{
		if (isGradualVectorDuplicate(multiple.listen, params))
			throw (ConfigException("error", 0, "Duplication listen's port", ""));
		return (true);
	}
	else if (nameId == SERVER_NAME)
	{
		if (isGradualVectorDuplicate(multiple.serverName, params))
			throw (ConfigException("error", 0, "Duplication server's name", ""));
		return (true);
	}
	else if (nameId == ERROR_PAGE)
	{
		std::vector<std::string>	tmp(params);
		tmp.erase(tmp.end() - 1);
		if (isGradualVectorDuplicate(multiple.errorPage, tmp))
			throw (ConfigException("error", 0, "Duplication error_page's code", ""));
		return (true);
	}
	else if (nameId == ACCEPTED_CGI_EXTENSION)
	{
		std::vector<std::string>	tmp(params);
		if (tmp.size() == 2)
			tmp.erase(tmp.end() - 1);
		if (isGradualVectorDuplicate(multiple.cgiPath, tmp))
			throw (ConfigException("error", 0, "Duplication cgi extentions", ""));
		return (true);
	}
	return (false);
};

static bool	isUniqSimpleDirective(const std::vector<SimpleDirective>& directives)
{
	int	nameId;
	std::vector<std::string>	names;

	for (size_t i = 0; i < directives.size(); ++i)
	{
		nameId = searchDirectiveName(directives[i].name);
		if (nameId == LISTEN || nameId == SERVER_NAME || nameId == ERROR_PAGE || nameId == ACCEPTED_CGI_EXTENSION)
		{
			if (hasUniqMultiParameter(nameId, directives[i].parameters))
            {
                MultipleParameter::cgiPath.clear();
				continue;
            }
		}
		else if (nameId == ACCEPTED_METHODS && hasDuplicate(directives[i].parameters))
			throw (ConfigException("error", 0, "Duplication accepted_method parameter", ""));
		else if (nameId == ROOT && std::find(names.begin(), names.end(), "alias") != names.end())
			throw (ConfigException("error", 0, "alias directive is difined earlier", "root " + directives[i].parameters[0]));
		else if (nameId == ALIAS && std::find(names.begin(), names.end(), "root") != names.end())
			throw (ConfigException("error", 0, "root directive is difined earlier", "alias " + directives[i].parameters[0]));
		if (std::find(names.begin(), names.end(), directives[i].name) != names.end())
			throw (ConfigException("error", 0, "Duplication '" + directives[i].name + "'", ""));
		names.push_back(directives[i].name);
	}
	clearMultipleParameter();
	return (true);
};

static const std::vector<BlockDirective>*	getServerBlocks(const MainDirective& main)
{
	return (&(main.blocks[0].blockDirectives));
}

static bool	isNoDuplication(const MainDirective& mainDir)
{
	if (1 < mainDir.blocks.size())
		throw (ConfigException("error", 0, "Multiple HTTP blocks", ""));
	isUniqSimpleDirective(mainDir.directives);
	isUniqSimpleDirective(mainDir.blocks[0].directives);
	for (size_t i = 0; i < getServerBlocks(mainDir)->size(); ++i)
	{
		std::vector<std::string>	location;
		std::vector<std::string>	serverName;

		isUniqSimpleDirective((*getServerBlocks(mainDir))[i].directives);
		for (size_t j = 0; j < (*getServerBlocks(mainDir))[i].blockDirectives.size(); ++j)
			isUniqSimpleDirective((*getServerBlocks(mainDir))[i].blockDirectives[j].directives);
		for (size_t j = 0; j < (*getServerBlocks(mainDir))[i].directives.size(); ++j)
		{
			if ((*getServerBlocks(mainDir))[i].directives[j].name == SIMPLE_DIRECTIVES[SERVER_NAME])
				if (isGradualVectorDuplicate(serverName, (*getServerBlocks(mainDir))[i].directives[j].parameters))
					throw (ConfigException("error", 0, "Duplication sever_name between server Context", ""));
		}
		for (size_t j = 0; j < (*getServerBlocks(mainDir))[i].blockDirectives.size(); ++j)
		{
			if ((*getServerBlocks(mainDir))[i].blockDirectives[j].nameContext == CONFIG_CONTEXTS[LOCATION])
			{
				location.push_back((*getServerBlocks(mainDir))[i].blockDirectives[j].parameter);
				if (hasDuplicate(location))
					throw (ConfigException("error", 0, "Duplication location between location Context", ""));
			}
		}
		location.clear();
	}
	return (true);
};

/**************
 * Parsing
************/

static int	getServerAmount(const MainDirective& mainDir)
{
	return (mainDir.blocks[0].blockDirectives.size());
};

static void	setErrorPage(std::map<int, std::string>& errorMap, const std::vector<std::string> src)
{
	for (size_t i = 0; i < src.size() - 1; ++i)
	{
		errorMap.insert(std::make_pair(to<int>(src[i]), src[src.size() - 1]));
	}
}

static void	setCgiExtension(std::map<std::string, std::string>& cgiMap, const std::vector<std::string> src)
{
	if (src.size() == 2)
		cgiMap.insert(std::make_pair(src[0], src[1]));
	else
		cgiMap.insert(std::make_pair(src[0], ""));
}

static void	setToLocationLevel(LocationDirective& lDir,
	const eSimpleDirective& target, const std::vector<std::string>& src)
{
	switch (target)
	{
	case ROOT:
		if (lDir.root.empty())
			lDir.root = src[0];
		break;
	case INDEX:
		if (lDir.index.empty())
			lDir.index = src[0];
		break;
	case ERROR_PAGE:
		setErrorPage(lDir.error_page, src);
		break;
	case AUTOINDEX:
		if (src[0] == "on")
			lDir.autoindex = true;
		else if (src[0] == "off")
			lDir.isSetAutoindex = true;
		break;
	case CLIENT_MAX_BODY_SIZE:
		if (lDir.isSetMaxBody == true)
			break;
		lDir.client_max_body_size = to<uint64>(src[0]);
		lDir.isSetMaxBody = true;
		break;
	case REDIRECT:
		if (lDir.redirect.empty())
			lDir.redirect = src[0];
		break;
	case ACCEPTED_CGI_EXTENSION:
		setCgiExtension(lDir.accepted_cgi_extension, src);
		break;
	case ACCEPTED_METHODS:
		if (lDir.accepted_methods.size() == 0)
			lDir.accepted_methods = src;
        if (std::find(lDir.accepted_methods.begin(), lDir.accepted_methods.end(), "GET") != lDir.accepted_methods.end())
            lDir.accepted_methods.push_back("HEAD");
		break;
	case ALIAS:
		if (lDir.alias.empty())
			lDir.alias = src[0];
		break;
	default:
		break;
	}
}

static void	setToServerLevel(ServerConfig& sConf, const eSimpleDirective& target, const std::vector<std::string>& src)
{
	// make random number for port
	std::srand(time(NULL));
	int portRand = std::rand() % 1000 + 8000;

	switch (target)
	{
	case LISTEN:
		if (src[0] == "xxx") //for test
			sConf.listens.push_back(portRand);
		else
			sConf.listens.push_back(to<uint16>(src[0]));
		break;
	case SERVER_NAME:
		sConf.server_names.push_back(src[0]);
		break;
	case UPLOAD_PATH:
		sConf.upload_path = src[0];
		break;
	default:
		break;
	}
}

static void	getFromLocationContext(const BlockDirective& server, ServerConfig& sConf)
{
	LocationDirective	tmp;

	for (size_t i = 0; i <= server.blockDirectives.size(); ++i)
	{
		sConf.locations.push_back(tmp);
		sConf.locations[i].client_max_body_size = 1000000;
		sConf.locations[i].isSetMaxBody = false;
		if (i == server.blockDirectives.size())
			return ;
		sConf.locations[i].location = server.blockDirectives[i].parameter;
		for (size_t j = 0; j < server.blockDirectives[i].directives.size(); ++j)
		{
			int	id = searchDirectiveName(server.blockDirectives[i].directives[j].name);
			setToLocationLevel(sConf.locations[i], static_cast<eSimpleDirective>(id), server.blockDirectives[i].directives[j].parameters);
		}
	}
}

static void	getFromServerContext(const BlockDirective& server, ServerConfig& sConf)
{
	for (size_t i = 0; i < server.directives.size(); ++i)
	{
		int	id = searchDirectiveName(server.directives[i].name);
		if (id == ERROR_PAGE)
		{
			for (size_t j = 0; j < sConf.locations.size() - 1; ++j)
				setErrorPage(sConf.locations[j].error_page, server.directives[i].parameters);
		}
		if (id == ACCEPTED_CGI_EXTENSION)
		{
			for (size_t j = 0; j < sConf.locations.size() - 1; ++j)
				setCgiExtension(sConf.locations[j].accepted_cgi_extension, server.directives[i].parameters);
		}
		if (id == AUTOINDEX && server.directives[i].parameters[0] == "on")
		{
			for (size_t j = 0; j < sConf.locations.size() - 1; ++j)
			{
				if (sConf.locations[j].isSetAutoindex == false)
					sConf.locations[j].autoindex = true;
			}
		}
		setToLocationLevel(sConf.locations[sConf.locations.size() - 1], static_cast<eSimpleDirective>(id), server.directives[i].parameters);
		setToServerLevel(sConf, static_cast<eSimpleDirective>(id), server.directives[i].parameters);
	}
}

static void	getFromMainContext(const MainDirective& mainDir, ServerConfig& sConf)
{
	if (!mainDir.directives.empty())
		sConf.error_log = mainDir.directives[0].parameters[0];
}

static void	getDefinedParam(const MainDirective& mainDir, ServerConfig& sConf)
{
	getFromLocationContext(mainDir.blocks[0].blockDirectives[sConf.serverID], sConf);
	getFromServerContext(mainDir.blocks[0].blockDirectives[sConf.serverID], sConf);
	getFromMainContext(mainDir, sConf);
}

static void	setUndefinedParam(ServerConfig& sConf)
{
	if (sConf.server_names.size() == 0)
		throw (ConfigException("error", 0, "server_name is not defined", ""));
	if (sConf.error_log.empty())
		sConf.error_log = "log/webserv.log";
	if (sConf.listens.empty())
		sConf.listens.push_back(8042);
	if (sConf.upload_path.empty())
		sConf.upload_path = "www/uploads/";
	for (size_t i = 0; i < sConf.locations.size(); ++i)
	{
		if (sConf.locations[i].root.empty())
			sConf.locations[i].root = "www/html";
		if (sConf.locations[i].index.empty())
			sConf.locations[i].index = "index.html";
		if (sConf.locations[i].accepted_methods.size() == 0)
        {
			sConf.locations[i].accepted_methods.push_back("GET");
			sConf.locations[i].accepted_methods.push_back("HEAD");
        }
	}
}

static void	setParamEachServer(const MainDirective& mainDir, std::vector<ServerConfig>& sConfs)
{
	ServerConfig	sConf;

	for (int i = 0; i < getServerAmount(mainDir); ++i)
	{
		sConfs.push_back(sConf);
		sConfs[i].serverID = i;
		getDefinedParam(mainDir, sConfs[i]);
		setUndefinedParam(sConfs[i]);
	}
};

static void	serverDuplicationCheck(const std::vector<ServerConfig>& sConfs)
{
	for (size_t i = 0; i < sConfs.size() - 1; ++i)
	{
		for (size_t j = 0; j < sConfs[i].server_names.size(); ++j)
		{
			if (std::find(sConfs[i + 1].server_names.begin(), sConfs[i + 1].server_names.end(), sConfs[i].server_names[j]) != sConfs[i + 1].server_names.end())
			{
				if (std::find(sConfs[i + 1].listens.begin(), sConfs[i + 1].listens.end(), sConfs[i].listens[j]) != sConfs[i + 1].listens.end())
					throw (ConfigException("error", 0, "Duplication server_name and listen", ""));
			}
		}
	}
};

std::string LocationDirective::translateURI(const std::string& uri) const
{
    if (alias.empty() == false)
        return alias + uri.substr(location.size());
    return RMV_LAST_SLASH(root) + uri;
}

bool LocationDirective::isMatching(const std::string& uri) const
{
    if (location.substr(0, 2) == "*.")
    {
        const std::string ext = location.substr(1);
        const std::string::size_type dotPos = uri.find(ext);

        if (dotPos == std::string::npos)
            return false;
        
        if (dotPos + ext.size() == uri.size())
            return true;

        if (uri[dotPos + ext.size()] == '/')
            return true;

        return false;
    }
    return uri.compare(0, location.size() > uri.size() ? uri.size() : location.size(), location) == 0;
}

const LocationDirective& ServerConfig::bestLocation(const std::string& uri)
{
    if (m_lastUri == uri)
        return locations[m_lastLocationIdx];
    
    m_lastUri = uri;

    std::map<uint32, std::vector<LocationDirective>::size_type> map;

    log << "requested URI: \"" << uri << "\"\n";

    for (std::vector<LocationDirective>::size_type i = 0; i != locations.size(); i++) 
    {
        if(locations[i].isMatching(uri))
        {
            if (locations[i].location.substr(0, 2) == "*.")
            {
                log << "using location: \"" << locations[i].location << "\"\n";
                m_lastLocationIdx = i;
                return locations[i];
            }
            map[locations[i].location.size()] = i;
            log << "matching location: \"" << locations[i].location << "\"\n";
        }
    }

    m_lastLocationIdx = (--map.end())->second;
    
    log << "using location: \"" << locations[(--map.end())->second].location << "\"\n";
    return locations[(--map.end())->second];
}

std::vector<ServerConfig>	parseServerConfig(const int ac, const char **av)
{
	try
	{
		std::string					fileContents;
		std::vector<Token>		 	tokens;
		MainDirective				mainDir;
		std::vector<ServerConfig>	sConfs;

		if (2 < ac)
			throw (ConfigException("error", 0, "Usage: " + std::string(av[0]) + " <config file>", ""));
		if (ac == 1)
			fileContents = readFileToString("configs/webserv.conf");
		else
			fileContents = readFileToString(av[1]);
		tokens = tokenize(fileContents);
		if (!tokens.size())
			throw (ConfigException("error", 0, "Configuration hasn't element.", ""));
		makeTmpStruct(tokens, mainDir, "main", 0, 0);
		isNoDuplication(mainDir);
		setParamEachServer(mainDir, sConfs);
		serverDuplicationCheck(sConfs);
		// std::cout << sConfs;
		return (sConfs);
	}
	catch(const ConfigException& e)
	{
		throw ;
	}
	catch(const std::exception& e)
	{
		throw (ConfigException("error", 0, std::string(e.what()), ""));
	}
}

/**************
 * Exception
************/
ConfigException::ConfigException(const std::string& level,
	const size_t& lineNo, const std::string& description, const std::string& token)
	:m_msg("[" + level + "] ")
{
	if (lineNo != 0)
		m_msg += "line#" + to_string<size_t>(lineNo) + " : ";
	m_msg += description;
	if (!token.empty())
		m_msg += ": " + token;
};

}
