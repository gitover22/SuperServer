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
std::string Buffer::RetrieveAllToStr(){
    std::string str(Peek(),ReadableBytes());
    RetrieveAll();
    return str;
}

const char* Buffer::BeginWriteConst() const{
    return BeginPtr_() + writePos;
}
char* Buffer::BeginWrite(){
    return BeginPtr_() +writePos;
}

void Buffer::HasWritten(size_t len){
    writePos += len;
}
void Buffer::Append(const std::string& str){
    Append(str.data(),str.length());
}
void Buffer::Append(const void *data,size_t len){
    assert(data);
    Append(static_cast<const char *>(data),len);
}
void Buffer::Append(const char *ss,size_t len){
    assert(ss);
    EnsureWriteable(len);
    std::copy(ss,ss+len,BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer&buff){
    Append(buff.Peek(),buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len){
    if(WritableBytes() < len){
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}

char* Buffer::BeginPtr_(){
    return &*buffer_.begin();
}
const char* Buffer::BeginPtr_() const{
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len){
    if(WritableBytes() + PrependableBytes() <len){
        buffer_.resize(writePos+len+1); // 扩容
    }else{
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos,BeginPtr_()+writePos,BeginPtr_());
        readPos =0;
        writePos =readPos+readable;
        assert(readable == ReadableBytes());
    }
}

ssize_t Buffer::WriteFd(int fd,int* saveErrno){
    size_t readSize =ReadableBytes();
    ssize_t len = write(fd,Peek(),readSize);
    if(len < 0){
        *saveErrno = errno;
        return len;
    }
    readPos += len;
    return len;
}

ssize_t Buffer::ReadFd(int fd ,int *saveErrno){
    char buff[65535];
    struct iovec iov[2]; // sys/uio.h
    const size_t writable =WritableBytes();
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = BeginPtr_() + writePos;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len =readv(fd,iov,2);
    if(len < 0){
        *saveErrno =errno;
    }else if(static_cast<size_t>(len) <= writable){
        writePos += len;
    }else{
        writePos = buffer_.size();
        Append(buff, len-writable);
    }
    return len;
}