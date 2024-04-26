/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndexResponse.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 16:50:25 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 14:37:44 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/RequestHandler.hpp"

#include <dirent.h>
#include <ctime>
#include <algorithm>
#include <sys/stat.h>

#include "Utils/Macros.hpp"
#include "Utils/Logger.hpp"

namespace webserv
{

struct DirecInfo
{
    std::string retPath;
    std::string fileSize;
    struct tm lastModified;
};

enum TmValue
{
    TM_SEC,
    TM_MIN,
    TM_HOUR,
    TM_MDAY,
    TM_MON,
    TM_YEAR,
    TM_WDAY,
    TM_YDAY,
    TM_ISDST,
    STRUCT_ENUM_SIZE
};

static DIR* opendirWrap(const std::string& path, int& responseCode)
{
    DIR* dir;

    dir = opendir(path.c_str());
    if (dir == NULL)
    {
        log << ("[error] opendir(): " + std::string(std::strerror(errno)));
        if (errno == EACCES)
            responseCode = 403;
        else if (errno == EMFILE || errno == ENFILE || errno == ENOMEM)
            responseCode = 500;
        else if (errno == ENOENT || errno == ENOTDIR)
            responseCode = 404;
    }
    return (dir);
}

static const std::string formatFileSize(off_t size)
{
    std::string unit[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;

    while (size >= 1024 && i < 4)
    {
        size /= 1024;
        ++i;
    }
    return (webserv::to_string(size) + unit[i]);
}

static std::vector<DirecInfo> readDirectoryInfo(DIR* dir, const std::string& confRoot, int& responseCode)
{
    struct dirent* diread;
    std::vector<DirecInfo> infos;
    struct stat stBuf;

    errno = 0;
    while ((diread = readdir(dir)) != NULL)
    {
        DirecInfo newInfo;
        newInfo.retPath = diread->d_name;
        if (diread->d_type == DT_DIR)
            newInfo.retPath += '/';
        if (stat((std::string(confRoot + '/' + newInfo.retPath)).c_str(), &stBuf) == -1)
        {
            responseCode = 500;
            break;
        }
        newInfo.fileSize = formatFileSize(stBuf.st_size);
        newInfo.lastModified = *(std::localtime(&(stBuf.st_mtime)));
        infos.push_back(newInfo);
    }
    if (errno != 0)
    {
        closedir(dir);
        responseCode = 500;
    }
    return (infos);
}

static bool isDot(const DirecInfo& info)
{
    return (info.retPath == "./");
}

static bool isDoubleDot(const DirecInfo& info)
{
    return (info.retPath == "../");
}

static void deleteSecretDirectory(std::vector<DirecInfo>& infos)
{
    infos.erase(std::remove_if(infos.begin(), infos.end(), isDot), infos.end());
    infos.erase(std::remove_if(infos.begin(), infos.end(), isDoubleDot), infos.end());
}

static std::string toStringWithLeadingZero(int value)
{
    std::ostringstream out;
    out << std::setw(2) << std::setfill('0') << value;
    return (out.str());
}

static int getTmData(const struct tm tm, const int flag)
{
    switch (flag)
    {
        case TM_SEC:
            return (tm.tm_sec);
        case TM_MIN:
            return (tm.tm_min);
        case TM_HOUR:
            return (tm.tm_hour);
        case TM_MDAY:
            return (tm.tm_mday);
        case TM_MON:
            return (tm.tm_mon + 1);
        case TM_YEAR:
            return (tm.tm_year + 1900);
        case TM_WDAY:
            return (tm.tm_wday);
        case TM_YDAY:
            return (tm.tm_yday);
        case TM_ISDST:
            return (tm.tm_isdst);
        default:
            return (-1);
    }
}

static std::string makeResponseBody(const std::vector<DirecInfo>& infos, const std::string& requestUri)
{
    std::string retHtml;

    retHtml.append("<html>\n");
    retHtml.append("<head>\n");
    retHtml.append("<title>Index of " + requestUri + "</title>\n");
    retHtml.append("<style>\n");
    retHtml.append("body { background-color: #f2f2f2; }\n");
    retHtml.append("h1 { color: #333; }\n");
    retHtml.append("table { width: 100%; border-collapse: collapse; }\n");
    retHtml.append("th, td { padding: 8px; text-align: left; }\n");
    retHtml.append("th { background-color: #ddd; }\n");
    retHtml.append("tr:nth-child(even) { background-color: #f2f2f2; }\n");
    retHtml.append("a { color: #0000FF; text-decoration: none; }\n");
    retHtml.append(".directory-icon { display: inline-block; width: 16px; height: 16px; background-image: url(https://fonts.gstatic.com/s/i/materialiconsoutlined/folder/v1/24px.svg); background-size: contain; }\n");
    retHtml.append(".file-icon { display: inline-block; width: 16px; height: 16px; background-image: url(https://fonts.gstatic.com/s/i/materialiconsoutlined/description/v1/24px.svg); background-repeat: no-repeat; background-size: contain; }\n");
	retHtml.append(".delete-button { display: inline-block; padding: 4px 8px; background-color: #FF0000; color: #FFFFFF; text-decoration: none; }\n");
    retHtml.append("</style>\n");
    retHtml.append("</head>\n");
    retHtml.append("<body>\n");
    retHtml.append("<h1>Index of " + requestUri + "</h1>\n");
    retHtml.append("<table>\n");
    retHtml.append("<tr><th>Name</th><th>Last Modified</th><th>Size</th><th>Action</th></tr>\n");
    for (size_t i = 0; i < infos.size(); ++i)
    {
        retHtml.append("<tr><td>");
        if (infos[i].retPath[infos[i].retPath.size() - 1] == '/')
            retHtml.append("<span class=\"directory-icon\"></span>");
        else
            retHtml.append("<span class=\"file-icon\"></span>");
        retHtml.append("<a href=\"" + infos[i].retPath + "\">" + infos[i].retPath + "</a></td>");
        retHtml.append("<td>" + toStringWithLeadingZero(getTmData(infos[i].lastModified, TM_YEAR)) + "-");
        retHtml.append(toStringWithLeadingZero(getTmData(infos[i].lastModified, TM_MON)) + "-");
        retHtml.append(toStringWithLeadingZero(getTmData(infos[i].lastModified, TM_MDAY)) + " ");
        retHtml.append(toStringWithLeadingZero(getTmData(infos[i].lastModified, TM_HOUR)) + ":");
        retHtml.append(toStringWithLeadingZero(getTmData(infos[i].lastModified, TM_MIN)) + "</td>");
        retHtml.append("<td>" + to_string(infos[i].fileSize) + "</td>");
        if (infos[i].retPath[infos[i].retPath.size() - 1] != '/')
            retHtml.append("<td><button class='delete-button' data-path='" + infos[i].retPath + "'>Delete</button></td>");
        retHtml.append("</tr>\n");
    }
    retHtml.append("</table>\n");
    retHtml.append("<script>\n");
    retHtml.append("document.addEventListener('DOMContentLoaded', () => {\n");
    retHtml.append("    document.querySelectorAll('.delete-button').forEach(button => {\n");
    retHtml.append("        button.addEventListener('click', function(e) {\n");
    retHtml.append("            e.preventDefault();\n");
    retHtml.append("            const filePath = this.getAttribute('data-path');\n");
    retHtml.append("            fetch(filePath, {\n");
    retHtml.append("                method: 'DELETE'\n");
    retHtml.append("            })\n");
    retHtml.append("            .then(response => {\n");
    retHtml.append("                if (response.ok) {\n");
    retHtml.append("                    console.log('File deleted successfully');\n");
    retHtml.append("                    location.reload();\n");
    retHtml.append("                } else {\n");
    retHtml.append("                    console.log('File deletion failed, status:', response.status);\n");
    retHtml.append("                    response.text().then(text => {\n");
    retHtml.append("                        console.log('Error response body:', text);\n");
    retHtml.append("                        document.body.innerHTML = text;\n");
    retHtml.append("                    })\n");
    retHtml.append("                }\n");
    retHtml.append("            })\n");
    retHtml.append("            .catch(error => {\n");
    retHtml.append("                console.error('Error:', error);\n");
    retHtml.append("            });\n");
    retHtml.append("        });\n");
    retHtml.append("    });\n");
    retHtml.append("});\n");
    retHtml.append("</script>\n");
    retHtml.append("</body>\n");
    retHtml.append("</html>\n");
    return retHtml;
}

void RequestHandler::makeAutoindexResponse(const std::string& uri)
{
    DIR* dir;
    int responseCode = 0;
    std::vector<DirecInfo> infos;

    if (!(dir = opendirWrap(m_location.root + uri, responseCode)))
        return makeErrorResponse(responseCode);

    infos = readDirectoryInfo(dir, m_location.root + uri, responseCode);
    if (responseCode != 0)
        return makeErrorResponse(responseCode);

    deleteSecretDirectory(infos);

    if (closedir(dir) == -1)
        return makeErrorResponse(500);

    for (size_t i = 0; i < infos.size(); ++i)
    {
        if (uri[uri.size() - 1] != '/')
            infos[i].retPath = '/' + infos[i].retPath;
        infos[i].retPath = uri + infos[i].retPath;
    }

    m_response->setStatusCode(200);
    m_response->body = to_vector(makeResponseBody(infos, uri));
    m_response->headers["Content-Type"] = "text/html";
    m_response->headers["Content-Length"] = to_string(m_response->body.size());
    m_response->isComplete = true;
}

} // namespace webserv
