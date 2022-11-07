#include <iostream>

#include <glog/logging.h>
#include <thread>
#include <csignal>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <fstream>
#include <sys/wait.h>
#include <set>

#include "CFileUtil.h"
#include "CLuaCfg.h"
#include "CValue.h"
#include "UdpServerBase.h"

using namespace std;
using namespace RC;

string version = "0.0.1";

int InitDaemon() {
    if (fork() != 0) {
        exit(0);
    }
    setsid();
    signal(SIGHUP, SIG_IGN);
    if (fork() != 0) {
        exit(0);
    }
    return 0;
}

static void signal_pipe_fun(int signal_type) {
}

inline bool getIpPort(string &v, string &ips, uint16_t &port) {
    size_t found = v.find(":");
    if (found == std::string::npos) {
        return false;
    }
    port = (uint16_t) atoi(v.substr(found + 1).c_str());
    ips = v.substr(0, found);
    return true;
}

void SplitString(const std::string &s, std::set<std::string> &_set, const std::string &c) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
        _set.insert(s.substr(pos1, pos2 - pos1));
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        _set.insert(s.substr(pos1));
}

void SplitString(const std::string &s, std::vector<std::string> &v, const std::string &c) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}


int main(int argc, char *argv[]) {
    int c;  //输入标记
    int help_flag = 0;
    string server_ip = "0.0.0.0";
    int server_port = 9001;
    bool is_daemon = false;
    bool debug = false;

    string config_path;
    string log_path;

    struct option longOpts[] =
    {
            {"ip",                   required_argument, 0,        'i'},
            {"port",                 required_argument, 0,        'p'},
            {"daemon",               no_argument,       0,        'd'},
            {"debug",                no_argument,       0,        'D'},
            {"log_path",             no_argument,       0,        'g'},
            {"version",              no_argument,       0,        'v'},
            {"config",               required_argument, 0,        'c'},
            {"help", 0,                                 &help_flag, 1},
            {0,      0,                                 0,        0}
    };

    //有：表示有参数，两个：表示参数可选
    while ((c = getopt_long(argc, argv, "i:p:dDvg:c:?", longOpts, nullptr)) != EOF) {
        switch (c) {
            case 'i':
                server_ip = optarg;
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'g':
                log_path = optarg;
                break;
            case 'd':
                is_daemon = true;
                break;
            case 'D':
                debug = true;
                break;
            case 'v':
                printf("version : [%s]\n", version.c_str());
                exit(0);
            case 'c':
                config_path = optarg;
                break;
            case '?':
                exit(0);
            default:
                break;
        }
    }
    if (help_flag) {
        fprintf(stderr, "Usage: rc_server [OPTION]\n%s",
                "\t-i, --ip             ip\n "
                "\t-p, --port           port\n "
                "\t-d, --daemon         server run daemon\n"
                "\t-g, --log_path         log path\n"
                "\t-D, --debug          debug mode\n"
                "\t-c, --config         config path\n"
                "\t-v, --version        version\n"
        );
        exit(0);
    }

    if (is_daemon) {
        InitDaemon();
    }
    signal(SIGPIPE, signal_pipe_fun);

    string exeName = CFileUtil::getFileName(argv[0]);
    google::InitGoogleLogging(exeName.c_str());  //参数为自己的可执行文件名
    google::SetLogDestination(google::GLOG_INFO, (log_path + "INFO").c_str());

    if (debug) {
        google::SetStderrLogging(google::INFO); //设置级别高于 google::INFO 的日志同时输出到屏幕
        FLAGS_colorlogtostderr = true;    //设置输出到屏幕的日志显示相应颜色
    }
    //google::SetLogDestination(google::ERROR,"log/error_");    //设置 google::ERROR 级别的日志存储路径和文件名前缀
    //google::SetLogDestination(google::INFO,LOGDIR"/INFO_"); //设置 google::INFO 级别的日志存储路径和文件名前缀
    //google::SetLogDestination(google::WARNING,LOGDIR"/WARNING_");   //设置 google::WARNING 级别的日志存储路径和文件名前缀
    //google::SetLogDestination(google::ERROR,LOGDIR"/ERROR_");   //设置 google::ERROR 级别的日志存储路径和文件名前缀
    FLAGS_logbufsecs = 0;        //缓冲日志输出，默认为30秒，此处改为立即输出
    FLAGS_max_log_size = 100;  //最大日志大小为 100MB
    FLAGS_stop_logging_if_full_disk = true;     //当磁盘被写满时，停止日志输出
    //google::SetLogFilenameExtension("91_");     //设置文件名扩展，如平台？或其它需要区分的信息
    google::InstallFailureSignalHandler();      //捕捉 core dumped
    //google::InstallFailureWriter(&FailureWriter);    //默认捕捉 SIGSEGV 信号信息输出会输出到 stderr，可以通过下面的方法自定义输出>方式


    UdpServerBase udpServerBase;
    udpServerBase.setIpPort(server_ip,server_port);
    udpServerBase.startServer();
    udpServerBase.join();
    return 0;
}
