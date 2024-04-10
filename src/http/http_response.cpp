#include "http_response.h"
// 静态成员变量初试化
const std::unordered_map<std::string,std::string> HttpResponse::SUFFIX_TYPE={
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};
const std::unordered_map<int,std::string> HttpResponse::CODE_STATUS={
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};
const std::unordered_map<int,std::string> HttpResponse::CODE_PATH{
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};



HttpResponse::HttpResponse(){
    code = -1;
    path = srcDir = "";
    isKeepAlive = false;
    mmFile = nullptr; 
    mmFileStat = { 0 };
}
HttpResponse::~HttpResponse(){
    UnmapFile();    
}

void HttpResponse::Init(const std::string& strDir,std::string& path,bool isKeepAlive,int code ){
    assert(srcDir !="");
    if(mmFile) UnmapFile();
    this->code =code;
    this->isKeepAlive = isKeepAlive;
    this->path =path;
    this->srcDir =srcDir;
    this->mmFile =nullptr;
    this->mmFileStat = {0};
}
void HttpResponse::MakeResponse(Buffer& buff){
    if(stat((srcDir + path).data(), &mmFileStat) < 0 || S_ISDIR(mmFileStat.st_mode)) {
        code = 404;
    }else if(!(mmFileStat.st_mode & S_IROTH)) {
        code = 403;
    }else if(code == -1) { 
        code = 200; 
    }
    ErrorHtml();
    AddStateLine(buff);
    AddHeader(buff);
    AddContent(buff);
}
void HttpResponse::UnmapFile(){
    if(mmFile) {
        munmap(mmFile, mmFileStat.st_size);
        mmFile = nullptr;
    } 
}
char* HttpResponse::File(){

}
size_t HttpResponse::FileLen() const{

}
void HttpResponse::ErrorContent(Buffer& buff,std::string message){

}
int HttpResponse::Code() const{

}
void HttpResponse::AddStateLine(Buffer &buff){

}
void HttpResponse::AddHeader(Buffer &buff){

}
void HttpResponse::AddContent(Buffer &buff){

}

void HttpResponse::ErrorHtml(){

}
std::string HttpResponse::GetFileType(){
    
}