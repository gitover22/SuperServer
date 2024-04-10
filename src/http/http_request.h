#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "../buffer/buffer.h"
#include <algorithm>
#include "../log/log.h"
#include <regex>
#include <mysql/mysql.h>
#include "../pool/sqlconnRAll.h"
class HttpRequest{
public:
    HttpRequest();
    ~HttpRequest() = default;
    // http请求报文的划分
    enum PARSE_STATE{
        REQUEST_LINE, // 请求行
        HEADERS,      // 请求头部
        BODY,         // 请求体
        FINISH,       //
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    void Init();

    bool parse(Buffer& buff);

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;

    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;
    bool IsKeepAlive() const;

private:
    /**
     * @brief 解析请求行
    */
    bool ParseRequestLine_(const std::string& line);
    /**
     * @brief 解析请求头部
    */
    void ParseHeader_(const std::string& line);
    /**
     * @brief 解析请求体
    */
    void ParseBody_(const std::string& line);
    /**
     * @brief 解析路径
    */
    void ParsePath_();
    void ParsePost_();
    void ParseFromUrlencoded_();

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);
    static int ConverHex(char ch);

    PARSE_STATE state_;
    std::string method_,version_, body_;
    std::string path_; // 代表html的路径
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;


};


#endif