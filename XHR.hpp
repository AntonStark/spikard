#ifndef XHR_H
#define XHR_H

#include <string>
#include <vector>
#include <map>
#include <sstream>

void lineToWords(std::string, std::string, std::vector<std::string>&);
void myToLower(std::string&);

class XHR {
private:
    std::vector<std::string> startLine;
    std::map<std::string, std::string> headers;
    bool content;
    std::string body;
public:
    XHR() {}
    XHR(std::string);
    XHR(const XHR& other) :
            startLine(other.startLine),
            headers(other.headers),
            content(other.content),
            body(other.body) {}
    XHR& operator= (const XHR other)
    {
        startLine = other.startLine;
        headers = other.headers;
        content = other.content;
        body = other.body;
        return *this;
    }
    ~XHR() {}
    operator std::string() const;

    void setL(std::string a, std::string b, std::string c)
    {
        startLine.resize(3);
        startLine[0] = a; startLine[1] = b; startLine[2] = c;
    }
    void setH(std::string key, std::string value)
        {headers[key] = value;}
    void setB(std::string source)
    {
        body = source;
        std::stringstream ss; ss << body.length(); std::string len = ss.str();
        setH("Content-Length", len);
        content = true;
    }
    std::vector<std::string> getL() const
        {return startLine;}
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
};

XHR confXHR(std::string);
void confXHR(XHR&, std::string);

#endif //XHR_H