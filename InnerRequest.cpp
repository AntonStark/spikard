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

inline std::vector<std::string> lineToWords(std::string line, std::string split)
{
    std::vector<std::string> words;
    lineToWords(line, split, words);
    return words;
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
    std::vector<std::string> parse = lineToWords(source, ";;");
    std::vector<std::string> head = lineToWords(parse[0], ";");

    auto it = head.begin();
    auto e = head.end();
    while (it != e)
    {
        std::vector<std::string> pair = lineToWords(*it, ": ");
        if (pair.size() != 2)
            break;
        myToLower(pair[0]);
        myToLower(pair[1]);
        headers[pair[0]] = pair[1];
        it++;
    }

    if (parse.size() > 1)
    {
        std::stringstream _body;
        auto pit = parse.begin()+1, pe = parse.end()-1;
        while (pit != pe)
        {
            _body << *pit << ";;";
            pit++;
        }
        _body << *pit;

        body = _body.str();
        content = true;
    }

    if (getH("user-id") == "")
        throw bad_req_ex("нет заголовка \"user-id\"");
    if (getH("length")  == "")
        throw bad_req_ex("нет заголовка \"length\"");
    if (content &&  atoi(getH("length").c_str()) != body.length() )
        throw bad_req_ex("getH(\"length\")!=body.length()");
    return;
}