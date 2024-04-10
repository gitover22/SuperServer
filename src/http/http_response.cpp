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
    return mmFile;
}
size_t HttpResponse::FileLen() const{
    return mmFileStat.st_size;
}
void HttpResponse::ErrorContent(Buffer& buff,std::string message){

}
int HttpResponse::Code() const{

}
void HttpResponse::AddStateLine(Buffer &buff){
    std::string status;
    if(CODE_STATUS.count(code) == 1){
        status = CODE_STATUS.find(code)->second;
    }else{
        code = 400;
        status =CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1"+std::to_string(code)+""+status+"\r\n");
}
void HttpResponse::AddHeader(Buffer &buff){
    buff.Append("Connection: ");
    if(isKeepAlive){
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");

    }else{
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType() + "\r\n");
}
void HttpResponse::AddContent(Buffer &buff){
    int srcFd = open((srcDir + path).data(), O_RDONLY);
    if(srcFd < 0) { 
        ErrorContent(buff, "File NotFound!");
        return; 
    }
    /* 将文件映射到内存提高文件的访问速度 
        MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    LOG_DEBUG("file path %s", (srcDir + path).data());
    int* mmRet = (int*)mmap(0, mmFileStat.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mmRet == -1) {
        ErrorContent(buff, "File NotFound!");
        return; 
    }
    mmFile = (char*)mmRet;
    close(srcFd);
    buff.Append("Content-length: " + std::to_string(mmFileStat.st_size) + "\r\n\r\n");

}

void HttpResponse::ErrorHtml(){
    if(CODE_PATH.count(code) == 1){
        path = CODE_PATH.find(code)->second;
        stat((srcDir+path).data(),&mmFileStat);
    }
}
std::string HttpResponse::GetFileType(){
    std::string::size_type idx =path.find_last_of('.');
    if(idx == std::string::npos){
        return "text/plaint";
    }
    std::string suffix =path.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1){
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";

}