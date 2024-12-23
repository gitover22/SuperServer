#include "../server/server.h"
int main(int argc ,char *argv[]){
    //
    std::cout<<"SuperServer is running..."<<std::endl;
    Server ss(1618,3,50000,false,3306,"huafeng","password","SuperServer",10,10,true,1,1024);
    ss.Start();
}