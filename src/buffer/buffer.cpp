#include "buffer.h"

// Buffer::Buffer(int initBufferSize = 1024){
//     buffer_ = std::vector<char>(initBufferSize);
//     readPos=0;
//     writePos=0;
// }
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos(0), writePos(0) {}

