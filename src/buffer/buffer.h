#ifndef BUFFER_H
#define BUFFER_H
class Buffer{
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;
    size_t WritableBytes() const; 
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    const char * Peek() const;
};





#endif
