#include "watchers.h"
#include "debug.h"

#include <arpa/inet.h>

void DataIoWatcher::setCliInfo(const sockaddr_in &cli_addr)
{
    ip_ = inet_ntoa(cli_addr.sin_addr);
    port_ = ntohs(cli_addr.sin_port);
}

const std::string &DataIoWatcher::ipCli() const noexcept
{
    return ip_;
}

uint16_t DataIoWatcher::portCli() const noexcept
{
    return port_;
}

std::string DataIoWatcher::cliInfo() const
{
    return ipCli() + ":" + std::to_string(portCli());
}

const std::string &DataIoWatcher::data() const noexcept
{
    return data_;
}

bool DataIoWatcher::operator<(const DataIoWatcher &r) const noexcept
{
    return io_watcher_.fd < r.io_watcher_.fd || io_watcher_.events < r.io_watcher_.events;
}

DataIoWatcher::DataIoWatcher(EV_P_ int fd, int events, std::string &&d)
    : io_watcher_(loop), data_(std::move(d)), port_(0)
{
    io_watcher_.set(fd, events);
}

DataIoWatcher::~DataIoWatcher()
{
    NORM("~IoDataWatcher_");
    io_watcher_.stop();
    if(io_watcher_.events == ev::READ){
        if (close(io_watcher_.fd)) {
            ERR("Error while closing fd: " << strerror(errno));
        }
    }
}

bool operator<(const pDataIoWatcher &l, const pDataIoWatcher &r)
{
    return *l < *r;
}

bool operator<(const pDataIoWatcher &l, const DataIoFd &r)
{
    return l->io_watcher_.fd < r.fd || l->io_watcher_.events < r.events;
}

bool operator<(const DataIoFd &l, const pDataIoWatcher &r)
{
    return l.fd < r->io_watcher_.fd || l.events < r->io_watcher_.events;
}
