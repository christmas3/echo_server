#include "echo_server.h"
#include "echo_server_impl.h"
#include "debug.h"

#include <regex>
#include <system_error>
#include <list>
#include <functional>

DEFINE_int32(port, 0, EchoServerImpl::portParamDescribe().c_str());
static bool validatePort(const char *flagname, int val)
{
    if (val >= EchoServerImpl::PORT_MIN() && val <= EchoServerImpl::PORT_MAX()) {
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

EchoServer::EchoServer(const std::string &ip_addr, uint16_t port)
    : up_himpl_{new EchoServerImpl{ip_addr, port}}
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
    NORM("Echo server on " << up_himpl_->servIp() << ":" << up_himpl_->servPort() << " started");
    up_himpl_->start();
}

void EchoServer::stop()
{
    up_himpl_->stop();
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

std::string EchoServer::destinyDescribe()
{
    return "Simple single-threaded echo TCP server";
}
