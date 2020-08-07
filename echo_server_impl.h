#ifndef ECHO_SERVER_IMPL_H
#define ECHO_SERVER_IMPL_H

#include "watchers_pre.h"

#include <ev++.h>

#include <string>
#include <memory>
#include <set>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

/// class of echo server implementation
class EchoServerImpl
{
public:
    /// Create echo server implementation
    /// \tparam IP template param for forwarding reference.
    /// \param ip_addr in symbolic format for server listening socket
    /// \param port for server listening socket
    template<typename IP>
    EchoServerImpl(IP &&ip_addr, uint16_t port)
        : serv_ip_(std::forward<IP>(ip_addr)), serv_port_(port), serv_addr_{0}, main_loop_(EV_DEFAULT),
          stop_watcher_(main_loop_), MAX_READ_(4096)
    {
        initStopWatcher_();
    }
    /// start to listening socket and add watcher to read from.
    /// run main event loop
    void start();
    /// send async event for main loop with stop watcher and call \ref stop_(ev::async, int)
    void stop();
    const std::string &servIp() const noexcept;
    std::uint16_t servPort() const noexcept;

    static std::uint16_t PORT_MIN() noexcept
    { return 55000; }
    static std::uint16_t PORT_MAX() noexcept
    { return 65535; }
    static std::string &portParamDescribe();

private:
    std::string serv_ip_;
    std::uint16_t serv_port_;
    sockaddr_in serv_addr_;
    struct ev_loop *main_loop_;
    const int family_ = AF_INET;
    ev::async stop_watcher_;
    const size_t MAX_READ_;
    ///contain all watchers for listening fd and input connection
    ///sorting wathers by fd number. std::less<> for searching by fd number
    std::set<std::shared_ptr<DataIoWatcher>, std::less<>> s_io_watchers_;
    using DataIoWatcherIter_ = std::set<std::shared_ptr<DataIoWatcher>>::iterator;
    DataIoWatcherIter_ findDataWatcherIter_(int fd);
    void deleteWatcher_(DataIoWatcherIter_ iter);
    void printConnectionCount_()const noexcept;

    void initStopWatcher_();
    void initSockaddrIn_();

    static std::uint32_t sinAddr_(int family, const std::string &ip_addr);
    /// watcher.data contain a pointer to EchoServerImpl object for access from ev::watchers callback.
    /// \tparam K ev::watcher type
    /// \param w ev::watcher reference
    /// \return pointer to EchoServerImpl object
    template<typename K>
    static EchoServerImpl *serverImpl(K &w)
    {
        return static_cast<EchoServerImpl *>(w.data);
    }
    /// Write data to socket fd
    /// \param fd socket fd
    /// \param data for write
    /// \return the number of written bytes
    static std::uint32_t writeTo_(int fd, const std::string &data);
    static bool setNonBlock_(int fd);

    ///ev::watchers callback
    static void stop_(ev::async &w, int revents);
    static void accept_(ev::io &w, int revents);
    static void read_write_(ev::io &w, int revents);
    static void write_(ev::io &w, int revents);

};

#endif // ECHO_SERVER_IMPL_H
