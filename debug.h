#ifndef DEBUG_H
#define DEBUG_H

#include <glog/logging.h>
#include <string.h>

#define C_RED "\33[1;31m"
#define C_GREEN     "\33[1;32m"
#define C_YELLOW "\33[1;33m"
//#define C_BLUE      "\33[1;34m"
//#define C_MAGENTA   "\33[1;35m"
#define C_END "\33[0;39m"

#define NORM(x) DLOG(INFO) << x;
#define DONE(x) DLOG(INFO) << C_GREEN << x << C_END;
#define WARN(x) DLOG(WARNING) << C_YELLOW << x << C_END;
#define ERR(x) DLOG(ERROR) << C_RED << x << C_END;
#define FATAL(x) LOG(FATAL) << C_RED << x << C_END;

#define THROW_SYS_ERR(x) \
    throw std::system_error(errno, \
                            std::system_category(), \
                            debug_func::errorMessage(x, __PRETTY_FUNCTION__, __LINE__))

namespace debug_func {
inline std::string errorMessage(const std::string &err, const std::string &func, int line_num)
{
    return "func: " + func + "; line: " + std::to_string(line_num) + "; mess: " + err + "; errno";
}
} // namespace debug_func

#endif // DEBUG_H
