#include "ConfigParser.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
#include <algorithm>
#include "Utils/Utils.hpp"
// #include <iostream> /*debug*/


/**************
 * Debug utils
************/

// void	printSimpleDirective(const SimpleDirective& simple)
// {
// 	std::cout << simple.name << ": ";
// 	printVector<std::string>(simple.parameters);
// };

// void	printBlockDirective(const BlockDirective& block)
// {
// 	std::cout << "-----" << block.nameContext << ": " << block.parameter << std::endl;
// 	for (size_t i = 0; i < block.directives.size(); ++i)
// 		printSimpleDirective(block.directives[i]);
// 	for (size_t i = 0; i < block.blockDirectives.size(); ++i)
// 		printBlockDirective(block.blockDirectives[i]);
// };

// void	printMainDirective(const MainDirective& main)
// {
// 	std::cout << "--- Main directives ---" << std::endl;
// 	for (size_t i = 0; i < main.directives.size(); ++i)
// 		printSimpleDirective(main.directives[i]);
// 	for (size_t i = 0; i < main.blocks.size(); ++i)
// 	{
// 		std::cout << "--- HTTP Blocks ---" << std::endl;
// 		printBlockDirective(main.blocks[i]);
// 	}
// };

// void	printServerConfig(const ServerConfig& sConf)
// {
// 	std::cout << "------ ServerConfig" << std::endl;
// 	std::cout << "error_log: " << sConf.error_log << std::endl;
// 	std::cout << "listens: ";
// 	printVector<int>(sConf.listens);
// 	std::cout << "server_names: ";
// 	printVector<std::string>(sConf.server_names);
// 	for (size_t i = 0; i < sConf.locations.size(); ++i)
// 	{
// 		std::cout << "------ location: ";
// 		std::cout << sConf.locations[i].location << std::endl;
// 		std::cout << "root: " << sConf.locations[i].root << std::endl;
// 		std::cout << "index: " << sConf.locations[i].index << std::endl;
// 		std::cout << "error_page: ";
// 		for (std::map<int, std::string>::const_iterator it = sConf.locations[i].error_page.begin(); it != sConf.locations[i].error_page.end(); ++it)
// 			std::cout << it->first << " => " << it->second << ", ";
// 		std::cout << std::endl;
// 		std::cout << "autoindex: " << sConf.locations[i].autoindex << std::endl;
// 		std::cout << "client_max_body_size: " << sConf.locations[i].client_max_body_size << std::endl;
// 		std::cout << "redirect: " << sConf.locations[i].redirect << std::endl;
// 		std::cout << "accepted_methods: ";
// 		printVector<std::string>(sConf.locations[i].accepted_methods);
// 		std::cout << std::endl;
// 	}
// };


/**************
 * readFileToString
************/
static std::string	readFileToString(const char *filePath)
{
	std::ifstream	ifs(filePath);
	std::string		fileContents;

	isFileAccessible(filePath);
	fileContents.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	ifs.close();
	return (fileContents);
};

/**************
 * Tokenize in Lexing
************/
static std::vector<std::string>	tokenize(const std::string& str)
{
	char		c;
	std::string					token;
	std::vector<std::string> 	tokens;

	for (size_t i = 0; i < str.length(); ++i)
	{
		c = str[i];
		throwIf(isCharInSet(c, SPECIAL_CHARS), std::string("Error: '") + c + "' can't usable.");
		if (std::isspace(c) || isCharInSet(c, DELIMITER_CHARS))
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
			if (isCharInSet(c, DELIMITER_CHARS))
				tokens.push_back(std::string(1, c));
		}
		else
			token += c;
	}
	if (!token.empty())
		tokens.push_back(token);
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
static bool	isValidDirective(const std::string& name, const std::string& parentContext)
{
	int	nameID = searchDirectiveName(name);
	int	parentID = searchContextName(parentContext);

	if (nameID == -1 || parentID == -1)
		return (false);
	if (parentID == MAIN 
			&& nameID == ERROR_LOG)
		return (true);
	else if (parentID == HTTP)
		return (false);
	else if (parentID == SERVER
			&& (LISTEN <= nameID && nameID <= INDEX))
		return (true);
	else if (parentID == LOCATION
			&& (ROOT <= nameID && nameID <= ACCEPTED_METHODS))
		return (true);
	return (false);
};

static bool	isValidBlock(const std::string& block, const std::string& parentContext)
{
	int	nameID = searchContextName(block);
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

static bool	isValidErrorPage(const std::vector<std::string>& tokens,
	int& index)
{
	std::string tmp;
	int 		errorCode;

	while (tokens[index + 1] != ";" && tokens[index] != ";" && (index < 0 || static_cast<uint64>(index) < tokens.size() - 1))
	{
		tmp = tokens[index];
		errorCode = convertStrToType<int>(tmp, isNumericLiteral);
		throwIf(errorCode < 200 || 599 < errorCode, "Error: invalid error code: " + tmp);
		tmp.clear();
		++index;
	}
	if (tokens[index + 1] == ";")
		return (true);
	return (false);
};

static bool	isValidAcceptedMethods(const std::vector<std::string>& tokens,
	int& index)
{
	std::string	tmp;

	while (tokens[index] != ";" && (index < 0 || static_cast<uint64>(index) < tokens.size() - 1))
	{
		tmp = tokens[index];
		if (tmp == "GET" || tmp == "POST" || tmp == "DELETE")
		{
			tmp.clear();
			if (tokens[index + 1] == ";")
				return (true);
			++index;
		}
		else
			throw (std::runtime_error("Error: invalid method: " + tmp));
	}
	return (false);
};

static bool	isValidParam(const std::vector<std::string>& tokens,
	int& index)
{
	std::string	param = tokens[index];
	int		nameID = searchDirectiveName(tokens[index - 1]);
	int 	port;
	int		size;

	if (isCharInSet(param[0], DELIMITER_CHARS))
		return (false);
	switch(nameID)
	{
		case ERROR_LOG:
			isFileAccessible(param.c_str());
			return (true);
		case LISTEN:
			if (param == "xxx") //for test
				return (true);	
			port = convertStrToType<int>(param, isNumericLiteral);
			if (port < 0 || port > 65535)
				return (false);
			return (true);
		case SERVER_NAME:
			return (true);
		case UPLOAD_PATH:
			return (true);
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
			size = convertStrToType<int>(param, isNumericLiteral);
			if (size < 0)
				return (false);
			return (true);
		case INDEX:
			return (true);
		case REDIRECT:
			return (true);
		case ACCEPTED_METHODS:
			if (!isValidAcceptedMethods(tokens, index))
				return (false);
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
	const std::vector<std::string>& tokens, int nameIndex, int semicoronIndex,
	const std::string& parentContext)
{
	SimpleDirective	simpleDir;

	simpleDir.name = tokens[nameIndex];
	for (int i = nameIndex + 1; i < semicoronIndex; ++i)
		simpleDir.parameters.push_back(tokens[i]);
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
	MainDirective& mainDir, const std::string& name, const std::string& locationName)
{
	int	parentID = searchContextName(parentContext);
	BlockDirective	blockDir;

	if (parentID == LOCATION)
	{
		blockDir.nameContext = locationName;
		blockDir.parameter = name;
		throwIf(isCharInSet(name[0], DELIMITER_CHARS), "Error: invalid location name: " + name);
	}
	else
	{
		blockDir.nameContext = name;
		blockDir.parameter = "";
	}
	if (parentID == HTTP)
		mainDir.blocks.push_back(blockDir);
	else if (parentID == SERVER)
		getLastHttpBlock(mainDir)->blockDirectives.push_back(blockDir);
	else if (parentID == LOCATION)
		getLastServerBlock(mainDir)->blockDirectives.push_back(blockDir);
};

static bool	isSimpleDirective(const std::vector<std::string>& tokens,
	MainDirective& mainDir, int& index, const std::string& parentContext)
{
	int			nameIndex;

	if (isValidDirective(tokens[index], parentContext))
	{
		nameIndex = index;
		++index;
		if (isValidParam(tokens, index))
		{
			++index;
			if (tokens[index] == ";")
			{
				addDirectiveToStruct(mainDir, tokens, nameIndex, index, parentContext);
				++index;
				return (true);
			}
		}
		throw (std::runtime_error("Unexpected token: " + tokens[index]));
	}
	return (false);
};

static bool	isBlockDirective(const std::vector<std::string>& tokens,
	MainDirective& mainDir, int& index, std::string& parentContext)
{
	if (isValidBlock(tokens[index], parentContext))
	{
		parentContext = tokens[index];
		if (tokens[index] == CONFIG_CONTEXTS[LOCATION])
			++index;
		++index;
		if (tokens[index] == "{")
		{
			addBlockToStruct(parentContext, mainDir, tokens[index - 1], tokens[index - 2]);
			++index;
			return (true);
		}
		throw (std::runtime_error("Unexpected token: " + tokens[index]));
	}
	return (false);
};

static bool	isEndOfBlock(const std::vector<std::string>& tokens,
	int& index, std::string& parentContext, int hasItem)
{
	if (searchContextName(parentContext) != MAIN
		&& 0 < hasItem
		&& tokens[index] == "}")
	{
		++index;
		resetParentContext(parentContext);
		return (true);
	}
	throw (std::runtime_error("Unexpected token: " + tokens[index]));
	return (false);
}

static bool	makeTmpStruct(const std::vector<std::string>& tokens,
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
		throwIf(hasItem == 0 || searchContextName(parentContext) != MAIN,
			"Error: Block is not closed or empty");
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
		throwIf(isGradualVectorDuplicate(multiple.listen, params),
			"Error: Duplication listen's port");
		return (true);
	}
	else if (nameId == SERVER_NAME)
	{
		throwIf(isGradualVectorDuplicate(multiple.serverName, params),
			"Error: Duplication server's name");
		return (true);
	}
	else if (nameId == ERROR_PAGE)
	{
		std::vector<std::string>	tmp(params);
		tmp.erase(tmp.end() - 1);
		throwIf(isGradualVectorDuplicate(multiple.errorPage, tmp), "Error: Duplication error_page's code");
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
		if (nameId == LISTEN || nameId == SERVER_NAME || nameId == ERROR_PAGE)
		{
			if (hasUniqMultiParameter(nameId, directives[i].parameters))
				continue;
		}
		else if (directives[i].name == SIMPLE_DIRECTIVES[ACCEPTED_METHODS])
			throwIf(hasDuplicate(directives[i].parameters), "Error: Duplication accepted_method parameter");
		if (std::find(names.begin(), names.end(), directives[i].name) != names.end())
			throw (std::runtime_error("Error: Duplication '" + directives[i].name + "'"));
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
	throwIf(1 < mainDir.blocks.size(), "Error: Multiple HTTP blocks");
	isUniqSimpleDirective(mainDir.directives);
	isUniqSimpleDirective(mainDir.blocks[0].directives);
	for (size_t i = 0; i < getServerBlocks(mainDir)->size(); ++i)
	{
		std::vector<std::string>	serverName;
		std::vector<std::string>	location;

		isUniqSimpleDirective((*getServerBlocks(mainDir))[i].directives);
		for (size_t j = 0; j < (*getServerBlocks(mainDir))[i].blockDirectives.size(); ++j)
			isUniqSimpleDirective((*getServerBlocks(mainDir))[i].blockDirectives[j].directives);
		for (size_t j = 0; j < (*getServerBlocks(mainDir))[i].directives.size(); ++j)
		{
			if ((*getServerBlocks(mainDir))[i].directives[j].name == SIMPLE_DIRECTIVES[SERVER_NAME])
				throwIf(isGradualVectorDuplicate(serverName, (*getServerBlocks(mainDir))[i].directives[j].parameters),
					"Error: Duplication sever_name between server Context");
		}
		for (size_t j = 0; j < (*getServerBlocks(mainDir))[i].blockDirectives.size(); ++j)
		{
			if ((*getServerBlocks(mainDir))[i].blockDirectives[j].nameContext == CONFIG_CONTEXTS[LOCATION])
			{
				location.push_back((*getServerBlocks(mainDir))[i].blockDirectives[j].parameter);
				if (hasDuplicate(location))
					throw (std::runtime_error("Error: Duplication location between location Context"));
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
		errorMap.insert(std::make_pair(convertStrToType<int>(src[i], isNumericLiteral), src[src.size() - 1]));
	}
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
		break;
	case CLIENT_MAX_BODY_SIZE:
		if (lDir.client_max_body_size == -1)
			lDir.client_max_body_size = convertStrToType<int>(src[0], isNumericLiteral);
		break;
	case REDIRECT:
		if (lDir.redirect.empty())
			lDir.redirect = src[0];
		break;
	case ACCEPTED_METHODS:
		if (lDir.accepted_methods.size() == 0)
			lDir.accepted_methods = src;
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
			sConf.listens.push_back(convertStrToType<int>(src[0], isNumericLiteral));
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
		sConf.locations[i].client_max_body_size = -1;
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
	static unsigned int i = 0;

	getFromLocationContext(mainDir.blocks[0].blockDirectives[i], sConf);
	getFromServerContext(mainDir.blocks[0].blockDirectives[i], sConf);
	getFromMainContext(mainDir, sConf);
	++i;
}

static void	setUndefinedParam(ServerConfig& sConf)
{
	throwIf(sConf.server_names.size() == 0, "Error: server_name is not defined");
	if (sConf.error_log.empty())
		sConf.error_log = "log/webserv.log";
	if (sConf.listens.empty())
		sConf.listens.push_back(8042);
	if (sConf.upload_path.empty())
		sConf.upload_path = "www/uploads";
	for (size_t i = 0; i < sConf.locations.size(); ++i)
	{
		if (sConf.locations[i].root.empty())
			sConf.locations[i].root = "www/html";
		if (sConf.locations[i].index.empty())
			sConf.locations[i].index = "index.html";
		if (sConf.locations[i].client_max_body_size == -1)
			sConf.locations[i].client_max_body_size = 1000000;
		if (sConf.locations[i].accepted_methods.size() == 0)
			sConf.locations[i].accepted_methods.push_back("GET");
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

std::vector<ServerConfig>	parseServerConfig(const int ac, const char **av)
{
	std::string					fileContents;
	std::vector<std::string> 	tokens;
	MainDirective				mainDir;
	std::vector<ServerConfig>	sConfs;

	throwIf(ac != 2, "Usage: " + std::string(av[0]) + " <config file>");
	fileContents = readFileToString(av[1]);
	tokens = tokenize(fileContents);
	makeTmpStruct(tokens, mainDir, "main", 0, 0);
	isNoDuplication(mainDir);
	setParamEachServer(mainDir, sConfs);
	// printServerConfig(sConfs[0]);
	return (sConfs);
}

/**************
 * Test main
************/

// int main(const int argc, const char **argv)
// {
// 	std::vector<ServerConfig>	sConfs = parseServerConfig(argc, argv);
// 	return (EXIT_SUCCESS);
// }
