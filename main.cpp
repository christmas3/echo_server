#include "debug.h"
#include "echo_server.h"

using namespace std;

int main(int argc, char** argv)
{
    int ret_code = 0;
    try {
        google::InitGoogleLogging(argv[0]);
        gflags::SetUsageMessage(EchoServer::destinyDescribe());
        gflags::ParseCommandLineFlags(&argc, &argv, true);

        EchoServer es{static_cast<std::uint16_t>(FLAGS_port)};

    } catch (std::system_error &e) {
        ERR(e.what());
//        ERR(e.what() << " errno: " << debug_func::errnoDescribe(e.code().value()));
        ret_code = e.code().value();
    }
    catch (std::exception &e) {
        ERR(e.what());
        ret_code = 1;
    }

    return ret_code;
}

