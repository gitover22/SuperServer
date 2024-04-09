#include "./http_request.h"

HttpRequest::HttpRequest(){
    Init();
}



void HttpRequest::Init(){

}

bool HttpRequest::parse(Buffer& buff){

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