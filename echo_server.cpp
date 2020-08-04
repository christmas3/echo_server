#include "echo_server.h"
#include "debug.h"

#include <ev++.h>

#include <regex>
#include <system_error>
#include <list>
#include <functional>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

DEFINE_int32(port, 0, EchoServer::portParamDescribe().c_str());
static bool validatePort(const char *flagname, int val)
{
    if (val >= EchoServer::PORT_MIN() && val <= EchoServer::PORT_MAX()) {
        return true;
    }
    ERR("Invalid value for server " << flagname << " = " << val);
    return false;
}
DEFINE_validator(port, &validatePort);

DEFINE_string(ip_addr, "*", "Server ip-address to listen. By default any address");
static bool validateIpAddr(const char *flagname, const std::string &val)
{
    std::regex re("^([0-9]{1,3}.){3}[0-9]{1,3}$");
    std::cmatch m;
    if (val == "*" || std::regex_match(val.c_str(), m, re)) {
        return true;
    }
    ERR("Invalid value for server " << flagname << " = " << val);
    return false;
}
DEFINE_validator(ip_addr, &validateIpAddr);

struct EchoServer::himpl_
{
    himpl_(const std::string &ip_addr, uint16_t port);
    void startListen();

    std::string ser_addr;
    std::uint16_t ser_port;
    int listen_fd;
    sockaddr_in serv_addr;
    struct ev_loop *main_loop;
    const int family = AF_INET;
    ev::async stop_watcher;
    std::list<ev::io> l_io_watchers;

private:
    void initSockaddrIn_();
    void stop_(ev::async &w, int revents);
    void accept_(ev::io &w, int revents);
    static std::uint32_t sinAddr_(int family, const std::string &ip_addr);
    typedef void (himpl_::*IoCb_)(ev::io &, int);
    void addListenWatcher_(int fd, int revents);
};

EchoServer::himpl_::himpl_(const std::string &ip_addr, uint16_t port)
    : ser_addr(ip_addr), ser_port(port), listen_fd{0},
      main_loop(EV_DEFAULT), stop_watcher(main_loop)
{
    stop_watcher.set<himpl_, &himpl_::stop_>(this);
    stop_watcher.start();
}

void EchoServer::himpl_::startListen()
{
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        THROW_SYS_ERR("Error call socket");
    }
    initSockaddrIn_();
    auto &&res = bind(listen_fd, (sockaddr *) &serv_addr, sizeof(serv_addr));
    if (res != 0) {
        THROW_SYS_ERR("Error call bind");
    }
    res = listen(listen_fd, 10);
    if (res != 0) {
        THROW_SYS_ERR("Error call listen");
    }
    addListenWatcher_(listen_fd, ev::READ);
}

void EchoServer::himpl_::initSockaddrIn_()
{
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = family;
    serv_addr.sin_addr.s_addr = sinAddr_(family, ser_addr);
    serv_addr.sin_port = htons(ser_port);
}

void EchoServer::himpl_::stop_(ev::async &w, int)
{
    for (auto &&w_io : l_io_watchers) {
        w_io.stop();
        if (close(w_io.fd)) {
            ERR("Error while closing fd: " << strerror(errno));
        }
    }
    l_io_watchers.clear();
    w.stop();
    ev_break(w.loop);
    NORM("Echo server stoped");
}

void EchoServer::himpl_::accept_(ev::io &w, int revents)
{
    NORM("accept !!!!");
}

uint32_t EchoServer::himpl_::sinAddr_(int family, const std::string &ip_addr)
{
    if (ip_addr == "*") {
        return htonl(INADDR_ANY);
    }
    in_addr in_ip;
    auto &&res = inet_pton(family, ip_addr.c_str(), &in_ip);
    if (res == -1) {
        THROW_SYS_ERR("Cannot convert " + ip_addr + " to binary");
    }
    if (res == 0) {
        throw std::logic_error("Error ip address format " + ip_addr);
    }
    return in_ip.s_addr;
}

void EchoServer::himpl_::addListenWatcher_(int fd, int revents)
{
    l_io_watchers.emplace_back(main_loop);
    l_io_watchers.back().set(fd, revents);
    l_io_watchers.back().set<himpl_,&himpl_::accept_>(this);
    l_io_watchers.back().start();
}

EchoServer::EchoServer(const std::string &ip_addr, uint16_t port)
    : up_himpl_{new himpl_{ip_addr, port}} {}

EchoServer& EchoServer::getEchoServer(const std::string &ip_addr, uint16_t port)
{
    static EchoServer up_echo_serv{ip_addr, port};
    return up_echo_serv;
}

EchoServer::~EchoServer()
{
    NORM("~EchoServer");
}

void EchoServer::start()
{
    up_himpl_->startListen();
    NORM("Echo server on " << up_himpl_->ser_addr << ":" << up_himpl_->ser_port << " started");
    ev_run(up_himpl_->main_loop);
}

void EchoServer::stop()
{
    ev_async_send(up_himpl_->main_loop, &up_himpl_->stop_watcher);
}

std::string &EchoServer::portParamDescribe()
{
    static std::string descr = "Server port number to listen(" + std::to_string(PORT_MIN()) + "-"
                               + std::to_string(PORT_MAX());
    return descr;
}

std::string EchoServer::destinyDescribe()
{
    return "Simple single-threaded echo TCP server";
}

std::vector<int> EchoServer::signalsForStop()
{
    return {SIGINT, SIGTERM, SIGQUIT, SIGSTOP, SIGTSTP};
}

void EchoServer::stopSignalHandler(int)
{
    for(auto&& sig : signalsForStop()){
        signal(sig, SIG_IGN);
    }
    EchoServer::getEchoServer().stop();
}
