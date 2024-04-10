#include "http_response.h"

HttpResponse::HttpResponse(){

}
HttpResponse::~HttpResponse(){

}

void HttpResponse::Init(const std::string& strDir,std::string& path,bool isKeepAlive = false,int code =-1){

}
void HttpResponse::MakeResponse(Buffer& buff){

}
void HttpResponse::UnmapFile(){

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