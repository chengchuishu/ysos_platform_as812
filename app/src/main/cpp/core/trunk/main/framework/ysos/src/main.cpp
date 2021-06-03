
#include "ysos.h"
#include <cstdio>
//#include <string>
#include <csignal>
#include <unistd.h>
#include <boost/thread.hpp>

bool stop_flag = false; 
void signal_callback_handler(int signum) {
    printf("Caught signal = [%d]\n", signum);
    stop_flag = true;
    exit(signum);
}

int main(int argc, char* argv[]) 
{
    // 设置信号回调函数，运行程序后键入ctrl + C回调signal_callback_handler函数
    signal(SIGINT, signal_callback_handler);
    printf("==========start service [check] ============\n");
    //TODO: first-  init ysoslog config and create all init log files
    int ret = theApp.InitInstance();
    printf("[********]mian[********][theApp.InitInstance()][ret][%d]\n", ret);
    //TODO: second- auto run all services
    if(!ret){
        printf("[********]mian[********][theApp.InitInstance()][fail][ret][%d]\n", ret);
    } else {
        printf("[********]mian[********][theApp.InitInstance()][success][ret][%d]\n", ret);
    }
    //boost::this_thread::sleep(boost::posix_time::seconds(2));
    ret = theApp.OnAutoRun();
    if(0 != ret) {
        printf("[********]mian[********][theApp.OnAutoRun()][fail][ret][%d]\n", ret);
    } else {
        printf("[********]mian[********][theApp.OnAutoRun()][success][ret][%d]\n", ret);   
        //signal(SIGTERM, signal_callback_handler);
        //  01.先运行该Link, 睡眠约50秒://NOLINT
        while (!stop_flag) {
            // boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(65000));
            boost::this_thread::sleep_for(boost::chrono::seconds(1000));
        }
        //TODO: exit app
        theApp.ExitInstance();
    }
    
    printf("==========start service [enter] ============\n");
    return 0;
}