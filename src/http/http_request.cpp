#include "./http_request.h"


// 静态成员变量
const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "/index", "/register", "/login",
    "/welcome", "/video", "/picture",
};
const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0}, 
    {"/login.html", 1}, 
};
HttpRequest::HttpRequest(){
    Init();
}



void HttpRequest::Init(){
    method_ = path_ =version_ =body_ ="";
    state_ =REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::parse(Buffer& buff){
    const char CRLF[] = "\r\n"; //http请求报文的每行结束标志
    if(buff.ReadableBytes() <=0)
        return false;   
    while(buff.ReadableBytes() && state_!=FINISH){
        // 根据结束标志找到lineEnd
        const char* lineEnd = std::search(buff.Peek(),buff.BeginWriteConst(),CRLF,CRLF+2);
        std::string line(buff.Peek(),lineEnd); // 截取的字符串
        // 根据state调用相应的处理函数
        switch(state_){
            case REQUEST_LINE:
                if(!ParseRequestLine_(line)){
                    return false;
                }
                ParsePath_();
                break;
            case HEADERS:
                ParseHeader_(line);
                if(buff.ReadableBytes() <=2){
                    // 请求体为空的情况
                    state_ =FINISH;
                }
                break;
            case BODY:
                ParseBody_(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.BeginWrite()) break;
        buff.RetrieveUntil(lineEnd+2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

std::string HttpRequest::path() const{

}
std::string& HttpRequest::path(){

}
std::string HttpRequest::method() const{

}
std::string HttpRequest::version() const{

}

std::string HttpRequest::GetPost(const std::string& key) const{

}
std::string HttpRequest::GetPost(const char* key) const{

}
bool HttpRequest::IsKeepAlive() const{

}

bool HttpRequest::ParseRequestLine_(const std::string& line){

}
void HttpRequest::ParseHeader_(const std::string& line){

}
void HttpRequest::ParseBody_(const std::string& line){

}

void HttpRequest::ParsePath_(){

}
void HttpRequest::ParsePost_(){

}
void HttpRequest::ParseFromUrlencoded_(){

}
bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool isLogin){

}