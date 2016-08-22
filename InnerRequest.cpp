#include "InnerRequest.hpp"

InnerRequest::InnerRequest(std::string source)
{
    std::vector<std::string> parse;
    lineToWords(source, ";", parse);

    std::vector<std::string>::iterator it = parse.begin();
    while (it != parse.end() && *it != "")
    {
        std::vector<std::string> pair;
        lineToWords(*it, ": ", pair);
        if (pair.size() == 2)
        {
            myToLower(pair[0]);
            myToLower(pair[1]);
            headers[pair[0]] = pair[1];
        }
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

InnerRequest::operator std::string() const
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