#include "buffer.h"
// 相比于函数内初试化，初试化列表开销更小
// Buffer::Buffer(int initBufferSize = 1024){
//     buffer_ = std::vector<char>(initBufferSize);
//     readPos=0;
//     writePos=0;
// }
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos(0), writePos(0) {}
/**
 * @brief 返回buffer中可读字节数
*/
size_t Buffer::ReadableBytes() const{
    return writePos - readPos;
}
/**
 * @brief 返回buffer中还可以写入的字节数
*/
size_t Buffer::WritableBytes()const{
    return buffer_.size() - writePos;
}

size_t Buffer::PrependableBytes() const{
    return readPos;
}
const char* Buffer::Peek() const {
    return BeginPtr_() +readPos;
}
void Buffer::Retrieve(size_t len){
    assert(len <= ReadableBytes());
    readPos += len;
}

void Buffer::RetrieveUntil(const char* end){
    assert(Peek() <= end);
    Retrieve(end - Peek());
}
/**
 * @brief 重置buffer区为全0
*/
void Buffer::RetrieveAll(){
    bzero(&buffer_[0],buffer_.size());
    readPos =0 ;
    writePos =0 ;
}
