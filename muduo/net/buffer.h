//
// Created by liucxi on 2022/10/24.
//

#ifndef MUDUO_BUFFER_H
#define MUDUO_BUFFER_H

#include <vector>
#include <string>
#include <sys/uio.h>
#include <cerrno>
#include <unistd.h>

/**
 * @brief 缓冲器类型定义
 * prepend-able bytes (8)
 * readable bytes           readIndex
 * writable bytes           writeIndex
 */
class Buffer {
public:
    static const size_t kCheapPrepend = 8;          // prepend-able bytes, 用来记录数据长度, 默认 8 字节
    static const size_t kInitSize = 1024;

    explicit Buffer(size_t initSize = kInitSize)
        : m_buffer(kCheapPrepend + initSize)
        , m_readIndex (kCheapPrepend)
        , m_writeIndex(kCheapPrepend) {
    };

    /// 可读数据长度
    size_t readableBytes() const { return m_writeIndex - m_readIndex; }

    /// 可写数据长度
    size_t writableBytes() const { return m_buffer.size() - m_writeIndex; }

    /// 标头数据长度
    size_t prependBytes() const { return m_readIndex; }

    /**
     * @brief 返回缓冲区可读数据的起始地址
     */
    const char *peek() const { return begin() + m_readIndex; }

    /**
     * @brief 把 onMessage 函数上报的 Buffer 数据转成 string 数据
     */
    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    /**
     * @brief 在读取完之后进行复位操作
     */
    void retrieve(size_t len) {
        if (len < readableBytes()) {
            m_readIndex += len;
        } else {
            retrieveAll();
        }
    }

    void retrieveAll() {
        m_readIndex = m_writeIndex = kCheapPrepend;
    }

    /**
     * @brief 确保可写空间足够
     */
    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    /**
     * @brief 将数据写入到缓冲区
     */
    void append(const char *data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        m_writeIndex += len;
    }

    /**
     * @brief 从 fd 上读取数据, poller 工作在 LT 模式
     */
    ssize_t readFd(int fd, int &saveErrno) {
        char extraBuf[65536] = {0};     // 栈上的空间大小, 64k

        struct iovec vec[2];
        const size_t writable = writableBytes();
        vec[0].iov_base = beginWrite();             // 第一块内存区域是缓冲区剩下的
        vec[0].iov_len = writable;
        vec[1].iov_base = extraBuf;                 // 第二块内存区域是栈上的
        vec[1].iov_len = sizeof extraBuf;

        const ssize_t n = ::readv(fd, vec, 2);
        if (n < 0) {
            saveErrno = errno;
        } else if (n <= writable) {
            m_writeIndex += n;
        } else {
            m_writeIndex = m_buffer.size();
            append(extraBuf, n - writable);
        }

        return n;
    }

    /**
     * @brief 通过 fd 发送数据
     */
    ssize_t writeFd(int fd, int &saveErrno) {
        ssize_t n = ::write(fd, peek(), readableBytes());
        if (n < 0) {
            saveErrno = errno;
        }
        return n;
    }
private:
    char *begin() { return &*m_buffer.begin(); }

    const char *begin() const { return &*m_buffer.begin(); }

    char *beginWrite() { return begin() + m_writeIndex; }

    const char *beginWrite() const { return begin() + m_writeIndex; }

    /**
     * @brief 进行空间扩容或者调整
     */
    void makeSpace(size_t len) {
        if (writableBytes() + prependBytes() < len + kCheapPrepend) {           // 现有空闲区域不够
            m_buffer.resize(m_writeIndex + len);    // 可读区域前面的部分就不考虑了，直接开辟需要的大小
        } else {                                                                // 现有空闲区域够，但是需要移动
            size_t readable = readableBytes();
            std::copy(begin() + m_readIndex, begin() + m_writeIndex, begin() + kCheapPrepend);
            m_readIndex = kCheapPrepend;
            m_writeIndex = m_readIndex + readable;
        }
    }

private:
    std::vector<char> m_buffer;
    size_t m_readIndex;
    size_t m_writeIndex;
};

#endif //MUDUO_BUFFER_H
