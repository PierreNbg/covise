#ifndef _HTTPCLIENT_CURL_HTTPMETHODS_H
#define _HTTPCLIENT_CURL_HTTPMETHODS_H

#include "export.h"
#include <string>
#include <curl/curl.h>

namespace opencover {
namespace httpclient {
namespace curl {

struct CURLHTTPCLIENTEXPORT HTTPMethod {
    HTTPMethod() = delete;
    ~HTTPMethod() = default;
    explicit HTTPMethod(const std::string &url): url(url) {}
    virtual void setupCurl(CURL *curl) const;
    virtual void cleanupCurl(CURL *curl) const;
    virtual const std::string to_string() const { return "URL: " + url + "\n"; }
    std::string url;
};

#define HTTP_METHOD(NAME) struct CURLHTTPCLIENTEXPORT NAME: public HTTPMethod

HTTP_METHOD(GET)
{
    GET() = delete;
    ~GET() = default;
    explicit GET(const std::string &url): HTTPMethod(url)
    {}
};

HTTP_METHOD(POST)
{
    POST() = delete;
    ~POST()
    {
        curl_slist_free_all(headers);
    }

    explicit POST(const std::string &url, const std::string &requestBody): HTTPMethod(url), requestBody(requestBody)
    {
        initHeaders();
    }

    // copy constructor
    POST(const POST &other): HTTPMethod(other.url), requestBody(other.requestBody)
    {
        initHeaders();
    }

    // copy assignment opertator
    POST &operator=(const POST &other)
    {
        if (this != &other)
            initHeaders();
        return *this;
    }

    void setupCurl(CURL * curl) const override;
    void cleanupCurl(CURL * curl) const override;
    const std::string to_string() const override
    {
        return HTTPMethod::to_string() + "Requestbody: " + requestBody + "\n";
    }
    std::string requestBody;

private:
    void initHeaders()
    {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    struct curl_slist *headers = nullptr;
};

// not implemented
//HTTP_METHOD(PUT, void);
//HTTP_METHOD(HEAD, void);
//HTTP_METHOD(DELETE, void);
//HTTP_METHOD(PATCH, void);
//HTTP_METHOD(OPTIONS, void);
//HTTP_METHOD(CONNECT, void);
//HTTP_METHOD(TRACE, void);

} // namespace curl
} // namespace httpclient
} // namespace opencover
#endif