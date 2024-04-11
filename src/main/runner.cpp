#include "../server/server.h"
int main(int argc ,char *argv[]){
    // TODO 构建Server对象
    Server ss(1618,3,50000,false,3306,"huafeng","huafeng","hf_db",10,10,true,1,1024);
    ss.Start();
}