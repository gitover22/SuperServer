#include "buffer.h"
// 开销更大
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

