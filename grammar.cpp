#include <iostream>
#include <map>
#include <string>
int main()
{
    std::multimap<int ,std::string> unmap;
    unmap.insert(std::make_pair<int,std::string>(12,"hello"));
    unmap.insert(std::make_pair<int,std::string>(12,"hello"));
    unmap.insert(std::make_pair<int,std::string>(32,"hello"));
    unmap.insert(std::make_pair<int,std::string>(12,"hello"));
    std::cout<<unmap.size()<<std::endl;
    for(auto& x :unmap){
            std::cout<<x.first<<" "<<x.second<<std::endl;        
    }
    return 0;
}