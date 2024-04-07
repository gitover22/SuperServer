#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
 
std::condition_variable cv;
std::mutex cv_m; // 此互斥用于三个目的：
                 // 1) 同步到 i 的访问
                 // 2) 同步到 std::cerr 的访问
                 // 3) 为条件变量 cv
int i = 0; // 共享资源
 
void waits()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::lock_guard<std::mutex> lk(cv_m);
    std::cout<<"test-----------"<<std::endl;
    std::cout<<"test-----------"<<std::endl;
    std::cout<<"test-----------"<<std::endl;
    std::cout<<"test-----------"<<std::endl;
    std::cout<<"test-----------"<<std::endl;
    std::cout<<"test-----------"<<std::endl;
    // std::cerr << "Waiting... \n";
    // cv.wait(lk, []{return i == 1;});
    // std::cerr << "...finished waiting. i == 1\n";
}
void func(){
    std::lock_guard<std::mutex> mm(cv_m);
    while(1){
        std::cout<<"bu"<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
// void signals()
// {
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     {
//         std::lock_guard<std::mutex> lk(cv_m);
//         std::cerr << "Notifying...\n";
//     }
//     cv.notify_all();
 
//     std::this_thread::sleep_for(std::chrono::seconds(1));
 
//     {
//         std::lock_guard<std::mutex> lk(cv_m);
//         i = 1;
//         std::cerr << "Notifying again...\n";
//     }
//     cv.notify_all();
// }
 
int main()
{
    std::thread t1(waits), t2(func);
    t1.join(); 
    t2.join(); 
    // t3.join();
    // t4.join();
}