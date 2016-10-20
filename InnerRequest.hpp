#ifndef INNER_REQUEST_HPP
#define INNER_REQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>

void lineToWords(std::string, std::string, std::vector<std::string>&);
void myToLower(std::string&);

class InnerRequest
{
private:
    std::map<std::string, std::string> headers;
    bool content;
    std::string body;
public:
    InnerRequest() :
            headers(),
            content(false),
            body() {}
    InnerRequest(const InnerRequest& other) :
            headers(other.headers),
            content(other.content),
            body(other.body) {}
    InnerRequest& operator= (const InnerRequest other)
    {
        headers = other.headers;
        content = other.content;
        body = other.body;
        return *this;
    }
    ~InnerRequest() {}

    void setH(std::string key, std::string value)
        {headers[key] = value;}
    void setB(std::string source)
    {
        body = source;

        int sourceLen = 0;
        auto it = body.begin(), e = body.end();
        while (it != e)
        {
            if ( (*it & 0xc0) != 0x80 ) //Магия! "Count all first-bytes (the ones that don't match 10xxxxxx)."
                ++sourceLen;
            it++;
        }
        std::stringstream ss; ss << sourceLen; std::string len = ss.str();
        setH("length", len);
        content = true;
    }
    std::string getH(std::string that) const
    {
        std::map<std::string, std::string>::const_iterator it = headers.find(that);
        if (it == headers.end() || it->second == "")
            return "";
        else
            return it->second;
    }
    std::string getB() const
        {return body;}

    std::map<std::string, std::string>::iterator firstH()
    {
        return headers.begin();
    };
    std::map<std::string, std::string>::iterator lastH()
    {
        return headers.end();
    };
    std::string toStr() const;
    void configure(std::string);
};

#endif //INNER_REQUEST_HPP