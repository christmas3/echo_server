#ifndef WATCHERS_H
#define WATCHERS_H

#include "watchers_pre.h"

#include <ev++.h>
#include <memory>

#include <netinet/in.h>

/// struct for compare DataIoWatcher with fd
struct DataIoFd
{
    int fd;
};

/// watcher that contains data and controls output by set/reset ev::WRITE
class DataIoWatcher
{
public:
    /// factory function
    /// \tparam method - function pointer to callback for ev::wathcer
    /// \tparam K - type of io::watcher::data member
    /// \param loop - for ev::watcher::loop member
    /// \param fd - for ev::watcher::fd member
    /// \param events - for ev::watcher::events member
    /// \param obj - pointer to object that contains in ev::watcher::data
    /// \param d - input data from clients
    /// \return smart pointer to DataIoWatcher.
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

    /// set client ip address and port from cli_addr
    /// \param cli_addr connection params from client
    void setCliInfo(const sockaddr_in &cli_addr);
    /// set new ev::watcher::events
    /// \param events
    void setEvents(int events);
    /// Add new data to the end of current data buf and set ev::WRITE event to send it back
    /// \param data - new data from client
    void addData(const std::string& data);
    /// Add number of last sended bytes of data to current.
    /// Clear data buf and sended count if new current number of sended bytes is equal to total data size
    /// \param sended - last sended bytes of data
    void addSended(std::uint32_t sended);

    const std::string &ipCli() const noexcept;
    std::uint16_t portCli() const noexcept;
    /// client info in format "ip:port"
    /// \return client info
    std::string cliInfo() const;
    /// data that didnt send to client back
    /// \return "all data buf" - "sended already"
    std::string data() const noexcept;
    int events()const noexcept{return io_watcher_.events;}

    bool operator<(const DataIoWatcher &r) const noexcept;

private:
    DataIoWatcher(EV_P_ int fd, int events, std::string &&d);

    ev::io io_watcher_;
    std::string data_;
    std::string ip_;
    std::uint16_t port_;
    std::uint64_t sended_;
    friend bool operator<(const pDataIoWatcher &l, const DataIoFd &r);
    friend bool operator<(const DataIoFd &l, const pDataIoWatcher &r);
};

bool operator<(const pDataIoWatcher &l, const DataIoFd &r);
bool operator<(const DataIoFd &l, const pDataIoWatcher &r);
bool operator<(const pDataIoWatcher &l, const pDataIoWatcher &r);

#endif // WATCHERS_H
