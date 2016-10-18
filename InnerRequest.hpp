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
    InnerRequest() {}
    //InnerRequest(std::string);
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
        std::stringstream ss; ss << body.length(); std::string len = ss.str();
        setH("length", len);
        content = true;
    }
    std::string getH(std::string that)
    {
        std::map<std::string, std::string>::iterator it = headers.find(that);
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