#include "XHR.h"

void lineToWords(std::string line, std::string split, std::vector<std::string>& words)
{
    size_t i, j;
    i = 0;
    while (true) {
        j = line.find(split, i);
        if (j == std::string::npos) {
            words.push_back(line.substr(i));
            break;
        }
        else {
            words.push_back(line.substr(i,j-i));
        }
        j += split.length();
        i = j;
    }
    return;
}

XHR::XHR(std::string source)
{
    std::vector<std::string> parse;
    lineToWords(source, "\r\n", parse);

    std::vector<std::string>::iterator it = parse.begin();
    lineToWords(*it, " ", startLine);
    if (startLine.size() < 3)
        startLine.resize(3);
    it++;
    while (it != parse.end() && *it != "")
    {
        std::vector<std::string> pair;
        lineToWords(*it, ": ", pair);
        headers[pair[0]] = pair[1];
        it++;
    }

    if (it != parse.end())
    {
        it++;
        if (it != parse.end())
        {
            body = *it;
            content = true;
        }
        else
            content = false;
    }
}

XHR::operator std::string() const
{
    std::stringstream buf;
    buf << startLine[0] << ' ' << startLine[1]
    << ' ' << startLine[2] << "\r\n";

    std::map<std::string, std::string>::const_iterator it = headers.begin();
    while (it != headers.end())
    {
        buf << it->first << ": " << it->second << "\r\n";
        it++;
    }
    buf << "\r\n";

    if (content)
        buf << body;
    buf<<"\0";
    return buf.str();
}

XHR confXHR(std::string source)
{
    XHR temp;
    temp.setL("HTTP/1.1", "200", "OK");
    temp.setH("Version", "HTTP/1.1");
    temp.setH("Content-Type", "text/html; charset=utf-8");
    temp.setH("Access-Control-Allow-Origin", "http://localhost:63342"/*"http://phoenix.spotlife.ru"*/);
    temp.setB(source);
    return temp;
}

void confXHR(XHR& target, std::string source)
{
    target.setL("HTTP/1.1", "200", "OK");
    target.setH("Version", "HTTP/1.1");
    target.setH("Content-Type", "text/html; charset=utf-8");
    target.setH("Access-Control-Allow-Origin", "http://localhost:63342"/*"http://phoenix.spotlife.ru"*/);
    target.setB(source);
    return;
}