#include "echo_server_impl.h"
#include "debug.h"
#include "echo_server.h"
#include "watchers.h"

#include <algorithm>

void EchoServerImpl::start()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        THROW_SYS_ERR("Error call socket");
    }
    initSockaddrIn_();
    auto &&res = bind(listen_fd, (sockaddr *) &serv_addr_, sizeof(serv_addr_));
    if (res != 0) {
        THROW_SYS_ERR("Error call bind");
    }
    res = listen(listen_fd, 10);
    if (res != 0) {
        THROW_SYS_ERR("Error call listen");
    }
    s_io_watchers_.insert(
        DataIoWatcher::create<&EchoServerImpl::accept_>(main_loop_, listen_fd, ev::READ, this, ""));
    ev_run(main_loop_);
}

void EchoServerImpl::stop()
{
    ev_async_send(main_loop_, &stop_watcher_);
}

const std::string &EchoServerImpl::servIp() const noexcept
{
    return serv_ip_;
}

uint16_t EchoServerImpl::servPort() const noexcept
{
    return serv_port_;
}

std::string &EchoServerImpl::portParamDescribe()
{
    static std::string descr = "Server port number to listen(" + std::to_string(PORT_MIN()) + "-"
        + std::to_string(PORT_MAX());
    return descr;
}

EchoServerImpl::DataIoWatcherIter_ EchoServerImpl::findDataWatcherIter_(int fd, int events)
{
    return s_io_watchers_.find(DataIoFd{fd, events});
}

void EchoServerImpl::deleteWatcher_(EchoServerImpl::DataIoWatcherIter_ iter)
{
    if (iter != s_io_watchers_.end()) {
        s_io_watchers_.erase(iter);
    }
    else {
        ERR("Cannot find IoDataWatcher");
    }
//    printConnectionCount_();
}

void EchoServerImpl::initStopWatcher_()
{
    stop_watcher_.set<&EchoServerImpl::stop_>();
    stop_watcher_.data = this;
    stop_watcher_.start();
}

void EchoServerImpl::initSockaddrIn_()
{
    bzero(&serv_addr_, sizeof(serv_addr_));
    serv_addr_.sin_family = family_;
    serv_addr_.sin_addr.s_addr = sinAddr_(family_, serv_ip_);
    serv_addr_.sin_port = htons(serv_port_);
}

uint32_t EchoServerImpl::sinAddr_(int family, const std::string &ip_addr)
{
    if (ip_addr == "*") {
        return htonl(INADDR_ANY);
    }
    in_addr in_ip{0};
    auto &&res = inet_pton(family, ip_addr.c_str(), &in_ip);
    if (res == -1) {
        THROW_SYS_ERR("Cannot convert " + ip_addr + " to binary");
    }
    if (res == 0) {
        throw std::logic_error("Error ip address format " + ip_addr);
    }
    return in_ip.s_addr;
}

void EchoServerImpl::stop_(ev::async &w, int)
{
    serverImpl(w)->s_io_watchers_.clear();
    w.stop();
    ev_break(w.loop);
    NORM("Echo server stoped");
}

void EchoServerImpl::accept_(ev::io &w, int)
{
    sockaddr_in cli_addr{0};
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
            auto *p_himpl = serverImpl(w);
            auto sp_data_watcher = DataIoWatcher::create<&EchoServerImpl::read_>(p_himpl->main_loop_,
                                                                                 cli_fd,
                                                                                 ev::READ,
                                                                                 p_himpl,
                                                                                 "");
            sp_data_watcher->setCliInfo(cli_addr);
            p_himpl->s_io_watchers_.insert(sp_data_watcher);
            DONE("Connected client = " << sp_data_watcher->ipCli() << ":"
                                       << sp_data_watcher->portCli());
//            p_himpl->printConnectionCount_();
        }
        catch (std::exception &e) {
            ERR(e.what());
            close(cli_fd);
            break;
        }
    }
}

void EchoServerImpl::read_(ev::io &w, int)
{
    try {
        auto *p_himpl = serverImpl(w);
        char buf[p_himpl->MAX_READ_];
        auto try_read{0};
        auto watcher_iter = p_himpl->findDataWatcherIter_(w.fd, ev::READ);
        while (try_read++ < 3) {
            auto &&readed = read(w.fd, buf, p_himpl->MAX_READ_);
            if (readed < 0) {
                if (errno == EINTR) {
                    WARN("read interrupt by system signal");
                    continue;
                }
                else {
                    ERR("Error readed from fd: " << strerror(errno));
                    p_himpl->deleteWatcher_(watcher_iter);
                }
            }
            else if (readed == 0) {
                WARN("Client disconnected = " << (*watcher_iter)->cliInfo());
                p_himpl->deleteWatcher_(watcher_iter);
            }
            else {
                p_himpl->s_io_watchers_.insert(
                    DataIoWatcher::create<&EchoServerImpl::write_>(p_himpl->main_loop_,
                                                                   w.fd,
                                                                   ev::WRITE,
                                                                   p_himpl,
                                                                   std::string(buf, readed)));
            }
            break;
        }
        if (try_read == 3) {
            ERR("Error while reading from clien = " << (*watcher_iter)->cliInfo());
        }
    }
    catch (std::exception &e) {
        ERR("Error while read from fd: " << e.what());
    }
}

void EchoServerImpl::write_(ev::io &w, int)
{
    try {
        auto *p_himpl = serverImpl(w);
        auto &&data_watcher_iter = p_himpl->findDataWatcherIter_(w.fd, ev::WRITE);
        writeTo_(w.fd, (*data_watcher_iter)->data());
        p_himpl->deleteWatcher_(data_watcher_iter);
    }
    catch (std::exception &e) {
        ERR("Error while write in fd: " << e.what());
    }
}
void EchoServerImpl::writeTo_(int fd, const std::string &data)
{
    size_t len_to_write = data.size();
    const char *cur_pos = data.c_str();
    while (len_to_write > 0) {
        auto &&writed = write(fd, cur_pos, len_to_write);
        if (writed < 0) {
            if (errno == EINTR) {
                WARN("write interruped by system signal");
                continue;
            }
            else {
                ERR("Error writed to fd: " << strerror(errno));
                break;
            }
        }
        len_to_write -= writed;
        cur_pos += writed;
    }
}
void EchoServerImpl::printConnectionCount_() const noexcept
{
    auto&& count = std::count_if(s_io_watchers_.begin(), s_io_watchers_.end(), [](const auto& sp_watcher)
    {
        return sp_watcher->events() == ev::READ;
    });
    NORM("Connections number = " << --count);
}

