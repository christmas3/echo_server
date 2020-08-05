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

    std::string serv_ip;
    std::uint16_t serv_port;
    sockaddr_in serv_addr;
    struct ev_loop *main_loop;
    const int family = AF_INET;
    ev::async stop_watcher;
    static constexpr size_t MAX_READ = 4096;

private:
    void initSockaddrIn_();
    void stop_(ev::async &w, int revents);
    static void accept_(ev::io &w, int revents);
    static void read_(ev::io &w, int revents);
    static void write_(ev::io &w, int revents);
    static std::uint32_t sinAddr_(int family, const std::string &ip_addr);
    template<void (*method)(ev::io &, int)>
    void addWatcher_(int fd, int revents, std::string &&data = "")
    {
        l_io_watchers_.push_back(pIoDataWatcher(new IoDataWatcher_(main_loop, std::move(data))));
        auto &iw = l_io_watchers_.back()->io_watcher;
        iw.set(fd, revents);
        iw.set<method>();
        iw.data = this;
        iw.start();
    }

    struct IoDataWatcher_
    {
        IoDataWatcher_(EV_P_ std::string &&d) : io_watcher(loop), data(std::move(d)), port(0){}
        void setCliInfo(const sockaddr_in &cli_addr)
        {
            ip = inet_ntoa(cli_addr.sin_addr);
            port = ntohs(cli_addr.sin_port);
        }
        ~IoDataWatcher_()
        {
            NORM("~IoDataWatcher_");
            io_watcher.stop();
            if (close(io_watcher.fd)) {
                ERR("Error while closing fd: " << strerror(errno));
            }
        }
        ev::io io_watcher;
        std::string data;
        std::string ip;
        std::uint16_t port;
    };
    using pIoDataWatcher = std::shared_ptr<IoDataWatcher_>;

    std::list<pIoDataWatcher> l_io_watchers_;
    using DataWatcherIter_ = std::list<pIoDataWatcher>::iterator;

    DataWatcherIter_ dataWatcherIter_(int fd);
    void deleteWatcher_(DataWatcherIter_ iter);
};

EchoServer::himpl_::himpl_(const std::string &ip_addr, uint16_t port)
    : serv_ip(ip_addr), serv_port(port),
      main_loop(EV_DEFAULT), stop_watcher(main_loop)
{
    stop_watcher.set<himpl_, &himpl_::stop_>(this);
    stop_watcher.start();
}

void EchoServer::himpl_::startListen()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
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
    addWatcher_<&himpl_::accept_>(listen_fd, ev::READ);
}

void EchoServer::himpl_::initSockaddrIn_()
{
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = family;
    serv_addr.sin_addr.s_addr = sinAddr_(family, serv_ip);
    serv_addr.sin_port = htons(serv_port);
}

void EchoServer::himpl_::stop_(ev::async &w, int)
{
    l_io_watchers_.clear();
    w.stop();
    ev_break(w.loop);
    NORM("Echo server stoped");
}

void EchoServer::himpl_::accept_(ev::io &w, int)
{
    sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int cli_fd = 0;
    while (cli_fd <= 0) {
        cli_fd = accept(w.fd, (sockaddr *) &cli_addr, &cli_len);
        if (cli_fd < 0) {
            if (errno == EINTR) {
                WARN("accept interrupted by system signal");
                continue;
            }
            ERR("Error accept call: " << strerror(errno));
            EchoServer::getEchoServer().stop();
        }
        try {
            auto *p_himpl = static_cast<himpl_*>(w.data);
            p_himpl->addWatcher_<&himpl_::read_>(cli_fd, ev::READ);
            auto& data_watch = p_himpl->l_io_watchers_.back();
            data_watch->setCliInfo(cli_addr);
            DONE("Connected client ip = " << data_watch->ip << " port = " << data_watch->port);
        } catch (std::exception &e) {
            ERR(e.what());
            close(cli_fd);
            break;
        }
    }
}

void EchoServer::himpl_::read_(ev::io &w, int)
{
    char buf[MAX_READ];
    ssize_t readed = 0;
    auto *p_himpl = static_cast<himpl_ *>(w.data);
    auto watcher_iter = p_himpl->dataWatcherIter_(w.fd);
    while (readed <= 0) {
        auto &&readed = read(w.fd, buf, MAX_READ);
        if (readed < 0) {
            if (errno == EINTR) {
                WARN("read interrupt by system signal");
                continue;
            } else {
                ERR("Error readed from fd: " << strerror(errno));
                break;
            }
        }
        else if(readed == 0){
            WARN("Client disconnected ip = " << (*watcher_iter)->ip
                                             << " port = " << (*watcher_iter)->port);
            p_himpl->deleteWatcher_(watcher_iter);
            break;
        } else {
            std::string data(buf, readed);
            NORM("Readed from client ip = " << (*watcher_iter)->ip << " port = " << (*watcher_iter)->port
                                            << " data = " << data);
        }
    }

}

//void EchoServer::himpl_::write_(ev::io &w, int revents)
//{

//}

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

void EchoServer::himpl_::deleteWatcher_(DataWatcherIter_ iter)
{
    if (iter != l_io_watchers_.end()) {
        l_io_watchers_.erase(iter);
    } else {
        ERR("Cannot find IoDataWatcher");
    }
}

EchoServer::himpl_::DataWatcherIter_ EchoServer::himpl_::dataWatcherIter_(int fd)
{
    return std::find_if(l_io_watchers_.begin(),
                        l_io_watchers_.end(),
                        [fd](const auto &data_watcher) {
                            return data_watcher->io_watcher.fd == fd;
                        });
}

EchoServer::EchoServer(const std::string &ip_addr, uint16_t port)
    : up_himpl_{new himpl_{ip_addr, port}}
{}

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
    NORM("Echo server on " << up_himpl_->serv_ip << ":" << up_himpl_->serv_port << " started");
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
