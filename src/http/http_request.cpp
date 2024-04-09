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
    return path_;
}
std::string& HttpRequest::path(){
    return path_;
}
std::string HttpRequest::method() const{
    return method_;
}
std::string HttpRequest::version() const{
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const{
    assert(key != "");
    if(post_.count(key) == 1){
        return post_.find(key)->second;
    }
    return "";
}
std::string HttpRequest::GetPost(const char* key) const{
    assert(key != nullptr);
    if(post_.count(key) == 1){
        return post_.find(key)->second;
    }
    return "";
}
bool HttpRequest::IsKeepAlive() const{
    if(header_.count("connection") == 1){
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HttpRequest::ParseRequestLine_(const std::string& line){
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,patten)){
        method_= subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_=HEADERS;
        return false;
    }
    LOG_ERROR("ParseRequestLine Error");
    return false;
}
void HttpRequest::ParseHeader_(const std::string& line){
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,patten)){
        header_[subMatch[1]] = subMatch[2];
    }else{
        state_ = BODY;
    }
}
void HttpRequest::ParseBody_(const std::string& line){
    body_ =line;
    ParsePost_();
    state_ =FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::ParsePath_(){
    if(path_ == "/"){
        path_ ="/index.html"; //用户第一次进入页面，导向index.html
    }else{
        for(auto &item:DEFAULT_HTML){
            if(item == path_){
                path_+= ".html";
                break;
            }    
        }
    }
}
void HttpRequest::ParsePost_(){
    if(method_ == "POST"&& header_["Content-Type"]=="application/x-www-form-urlencoded"){
        ParseFromUrlencoded_();
        if(DEFAULT_HTML_TAG.count(path_)){
            int tag =DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d",tag);
            if(tag == 0||tag ==1){
                bool isLogin =(tag==1);
                if(UserVerify(post_["username"],post_["password"],isLogin)){
                    path_ = "/welcome.html";
                }else{
                    path_ = "/error.html";
                }
            }
        }
    }
}
void HttpRequest::ParseFromUrlencoded_(){

}
bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool isLogin){

}