#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

#include <gflags/gflags.h>
#include <cstdint>
#include <memory>

DECLARE_int32(port);
DECLARE_string(ip_addr);

class EchoServerImpl;

///simple single-thread tcp echo server
class EchoServer
{
public:
    /// Create static EchoServer object if doesnt exist
    /// \param ip_addr for listen, or * if any
    /// \param port for listen
    /// \return link on static EchoServer object
    static EchoServer& getEchoServer(const std::string &ip_addr = "",
                                                  std::uint16_t port = 0);
    ~EchoServer();
    ///start listening socket and block
    void start();
    ///close all connections and listening socket and unblock
    void stop();

    /// extern system signals for stop server
    /// \return vector of signals number
    static std::vector<int> signalsForStop();
    /// systems signal handler for stop server
    static void stopSignalHandler(int);
    /// short describe server for --help command line param
    static std::string destinyDescribe();

    EchoServer(EchoServer &) = delete;
    EchoServer(EchoServer &&) = delete;
    EchoServer &operator=(const EchoServer &) = delete;
    EchoServer &operator=(EchoServer &&) = delete;
private:
    EchoServer(const std::string& ip_addr, std::uint16_t port);

    /// echo server implementation
    std::unique_ptr<EchoServerImpl> up_himpl_;
}; //class EchoServer

#endif // ECHO_SERVER_H
