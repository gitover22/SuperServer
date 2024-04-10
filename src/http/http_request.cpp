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
    if(body_.size() == 0) return ;
    std::string key,value;
    int num =0;
    int n = body_.size();
    int i =0,j=0;
    for(;i<n;i++){
        char ch =body_[i];
        switch(ch){
            case '=':
                key = body_.substr(j,i-j);
                j = i+1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
                body_[i + 2] = num % 10 + '0';
                body_[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    }
}
bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool isLogin){
    if(name=="" || pwd =="") return false;
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str()); // c_str(): string to char *
    MYSQL *sql;
    SqlConnRAII(&sql,SqlConnPool::Instance());
    assert(sql);
    bool flag = false;
    unsigned int j =0 ;
    char order[256] = {0};
    MYSQL_FIELD *fields=nullptr;
    MYSQL_RES *res= nullptr;
    if(!isLogin) flag = true;
    // 将SQL查询语句写入order中
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s",order);

    // mysql_query()执行SQL语句
    if(mysql_query(sql ,order)){
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);
    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string password(row[1]);
        if(isLogin) {
            if(pwd == password) { flag = true; }
            else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        } 
        else { 
            flag = false; 
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);
    // 注册
    if(!isLogin && flag == true) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "Insert error!");
            flag = false; 
        }
        flag = true;
    }
    SqlConnPool::Instance()->FreeConn(sql);
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}