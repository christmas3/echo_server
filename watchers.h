#ifndef WATCHERS_H
#define WATCHERS_H

#include "watchers_pre.h"

#include <ev++.h>
#include <memory>

#include <netinet/in.h>

struct DataIoFd
{
    int fd;
    int events;
};

class DataIoWatcher
{
public:
    template<void (*method)(ev::io &, int), typename K>
    static std::shared_ptr<DataIoWatcher> create(EV_P_ int fd, int events, K *obj, std::string &&d)
    {
        std::shared_ptr<DataIoWatcher> sp_data_watcher(
            new DataIoWatcher(loop, fd, events, std::move(d)));
        sp_data_watcher->io_watcher_.set<method>();
        sp_data_watcher->io_watcher_.data = obj;
        sp_data_watcher->io_watcher_.start();
        return sp_data_watcher;
    }
    ~DataIoWatcher();

    void setCliInfo(const sockaddr_in &cli_addr);

    const std::string &ipCli() const noexcept;
    std::uint16_t portCli() const noexcept;
    std::string cliInfo() const;
    const std::string &data() const noexcept;
    int events()const noexcept{return io_watcher_.events;}

    bool operator<(const DataIoWatcher &r) const noexcept;

private:
    DataIoWatcher(EV_P_ int fd, int events, std::string &&d);

    ev::io io_watcher_;
    std::string data_;
    std::string ip_;
    std::uint16_t port_;
    friend bool operator<(const pDataIoWatcher &l, const DataIoFd &r);
    friend bool operator<(const DataIoFd &l, const pDataIoWatcher &r);
};

bool operator<(const pDataIoWatcher &l, const DataIoFd &r);
bool operator<(const DataIoFd &l, const pDataIoWatcher &r);
bool operator<(const pDataIoWatcher &l, const pDataIoWatcher &r);

#endif // WATCHERS_H
