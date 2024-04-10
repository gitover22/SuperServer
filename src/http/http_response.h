#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <bits/struct_stat.h>
#include <string>
#include "../buffer/buffer.h"
#include <unordered_map>
#include <sys/mman.h>    // mmap, munmap
#include "sys/stat.h"
class HttpResponse{
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& strDir,std::string& path,bool isKeepAlive = false,int code =-1);
    void MakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff,std::string message);
    int Code() const;
private:
    void AddStateLine(Buffer &buff);
    void AddHeader(Buffer &buff);
    void AddContent(Buffer &buff);

    void ErrorHtml();
    std::string GetFileType();
    
    int code;
    bool isKeepAlive;
    std::string path;
    std::string srcDir;
    char* mmFile;
    struct stat mmFileStat;
    static const std::unordered_map<std::string,std::string> SUFFIX_TYPE;
    static const std::unordered_map<int,std::string> CODE_STATUS;
    static const std::unordered_map<int,std::string> CODE_PATH;



};


#endif