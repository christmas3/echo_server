#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

#include <gflags/gflags.h>
#include <cstdint>
#include <memory>

DECLARE_int32(port);

class EchoServer
{
public:
    EchoServer(std::uint16_t port);
    ~EchoServer();

    static std::uint16_t PORT_MIN() noexcept { return 55000; }
    static std::uint16_t PORT_MAX() noexcept { return 65535; }
    static std::string &portParamDescribe();
    static std::string destinyDescribe();

    EchoServer(EchoServer &) = delete;
    EchoServer(EchoServer &&) = delete;
    EchoServer &operator=(const EchoServer &) = delete;
    EchoServer &operator=(EchoServer &&) = delete;
private:
    struct himpl_;
    std::unique_ptr<himpl_> up_himpl_;
}; //class EchoServer

#endif // ECHO_SERVER_H
