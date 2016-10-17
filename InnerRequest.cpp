#include "InnerRequest.hpp"

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

void myToLower(std::string& source)
{
    std::locale loc;
    std::string::iterator it = source.begin(), e = source.end();
    while (it != e)
    {
        *it = std::tolower(*it, loc);
        ++it;
    }
    return;
}

std::string InnerRequest::toStr() const
{
    std::stringstream buf;

    std::map<std::string, std::string>::const_iterator it = headers.begin();
    while (it != headers.end())
    {
        buf << it->first << ": " << it->second << ";";
        it++;
    }
    buf << ";";

    if (content)
        buf << body;
    buf<<"\0";
    return buf.str();
}

void InnerRequest::configure(std::string source)
{
    std::vector<std::string> parse;
    lineToWords(source, ";", parse);

    std::vector<std::string>::iterator it = parse.begin();
    while (it != parse.end() && *it != "")
    {
        std::vector<std::string> pair;
        lineToWords(*it, ": ", pair);
        if (pair.size() != 2)
            break;
        myToLower(pair[0]);
        myToLower(pair[1]);
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