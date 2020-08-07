#include "echo_server.h"
#include "debug.h"

#include <signal.h>

int main(int argc, char** argv)
{
    int ret_code = 0;
    try {
        google::InitGoogleLogging(argv[0]);
        gflags::SetUsageMessage(EchoServer::destinyDescribe());
        gflags::ParseCommandLineFlags(&argc, &argv, true);

        auto &&es = EchoServer::getEchoServer(FLAGS_ip_addr, static_cast<std::uint16_t>(FLAGS_port));
        for (auto &&sig : EchoServer::signalsForStop()) {
            signal(sig, &EchoServer::stopSignalHandler);
        }
        es.start();

    } catch (std::system_error &e) {
        ERR(e.what());
        ret_code = e.code().value();
    }
    catch (std::exception &e) {
        ERR(e.what());
        ret_code = 1;
    }

    exit(ret_code);
}

