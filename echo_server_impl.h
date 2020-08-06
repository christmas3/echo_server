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

class EchoServerImpl
{
public:
    template<typename IP>
    EchoServerImpl(IP &&ip_addr, uint16_t port)
        : serv_ip_(std::forward<IP>(ip_addr)), serv_port_(port), serv_addr_{0}, main_loop_(EV_DEFAULT),
          stop_watcher_(main_loop_), MAX_READ_(4096)
    {
        initStopWatcher_();
    }
    void start();
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
    std::set<std::shared_ptr<DataIoWatcher>, std::less<>> s_io_watchers_;
    using DataIoWatcherIter_ = std::set<std::shared_ptr<DataIoWatcher>>::iterator;
    DataIoWatcherIter_ findDataWatcherIter_(int fd, int events);
    void deleteWatcher_(DataIoWatcherIter_ iter);
    void printConnectionCount_()const noexcept;

    void initStopWatcher_();
    void initSockaddrIn_();

    static std::uint32_t sinAddr_(int family, const std::string &ip_addr);
    template<typename K>
    static EchoServerImpl *serverImpl(K &w)
    {
        return static_cast<EchoServerImpl *>(w.data);
    }

    static void stop_(ev::async &w, int revents);
    static void accept_(ev::io &w, int revents);
    static void read_(ev::io &w, int revents);
    static void write_(ev::io &w, int revents);
    static void writeTo_(int fd, const std::string &data);
};

#endif // ECHO_SERVER_IMPL_H
