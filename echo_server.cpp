#include "echo_server.h"
#include "debug.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <system_error>

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

struct EchoServer::himpl_
{
    himpl_(uint16_t port);
    std::uint16_t ser_port;
    int listen_fd;
    sockaddr_in serv_addr;
};

EchoServer::himpl_::himpl_(uint16_t port)
    : ser_port(port) /*, listen_fd{socket(AF_INET, SOCK_STREAM, 0)}*/
{
    errno = E2BIG;
    THROW_SYS_ERR("Error call socket");
    //    if (listen_fd < 0) {
    //        THROW_SYS_ERR("Error call socket");
    //    }
}

EchoServer::EchoServer(uint16_t port) : up_himpl_{new himpl_{port}} {}

EchoServer::~EchoServer()
{
    NORM("~EchoServer");
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

