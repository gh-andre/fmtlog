#include <chrono>
#include <iostream>
#include <string_view>
#include <optional>
#include <thread>

#include "../fmtlog.h"

using namespace std::literals::string_view_literals;

void runBenchmark();

void logcb(int64_t ns, fmtlog::LogLevel level, fmt::string_view location, size_t basePos, fmt::string_view threadName,
           fmt::string_view msg, size_t bodyPos, size_t logFilePos) {
  fmt::print("callback full msg: {}, logFilePos: {}\n", msg, logFilePos);
  msg.remove_prefix(bodyPos);
  fmt::print("callback msg body: {}\n", msg);
}

std::optional<std::string_view> msgCB(std::string_view msg, std::string& msgCBStr, void* userData)
{
    size_t start = 0, pos = 0;

    std::string *newmsg = nullptr;
    
    while(start < msg.length() && (pos = msg.find_first_of("\r\n"sv, start)) != std::string_view::npos) {
        if(!newmsg) {
            newmsg = &msgCBStr;
            newmsg->clear();
        }

        // add the chunk between current starting position and the filtered character
        if(pos > start)
            newmsg->append(msg.data()+start, pos-start);

        // keep replacements distinct, so they can be converted back if needed
        switch(*(msg.data()+pos)) {
            case '\r':
                *newmsg += "_\\r_"sv;
                break;
            case '\n':
                *newmsg += "_\\n_"sv;
                break;
        }
        
        start = pos+1;
    }

    if(newmsg) {
        // if there's last segment, append it
        if(start < msg.length())
            newmsg->append(msg.data()+start, msg.length()-start);
    }

    // must construct string view explicitly, or a bad string view is created for a temporary optional<string>
    return newmsg ? std::make_optional<std::string_view>(*newmsg) : std::nullopt;
}

void logQFullCB(void* userData) {
  (*reinterpret_cast<size_t*>(userData))++;
  fmt::print("log q full\n");

  #if FMTLOG_BLOCK==1
  // wait for a few seconds to make it visible
  fmt::print("Waiting for 5 seconds...\n");
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // flush the log to continue (must be synchronized in real apps)
  std::thread t(&fmtlog::poll, true);
  t.join();
  #endif
}

int main() {
  // record a different log location than that of the log line
  FMTLOG_ONCE_LOCATION(fmtlog::INF, "some-other-file.cpp:123", "ABC {:d}", 123);

  fmtlog::poll();

  char randomString[] = "Hello World";
  logi("A string, pointer, number, and float: '{}', {}, {}, {}", randomString, (void*)&randomString,
       512, 3.14159);

  int a = 4;
  auto sptr = std::make_shared<int>(5);
  auto uptr = std::make_unique<int>(6);
  logi("void ptr: {}, ptr: {}, sptr: {}, uptr: {}", (void*)&a, &a, sptr, std::move(uptr));
  a = 7;
  *sptr = 8;

  char strarr[10] = "111";
  char* cstr = strarr;
  std::string str = "aaa";
  logi("str: {}, pstr: {}, strarr: {}, pstrarr: {}, cstr: {}, pcstr: {}", str, &str, strarr, &strarr, cstr, &cstr);
  str = "bbb";
  strcpy(cstr, "222");

  // logi(FMT_STRING("This msg will trigger compile error: {:d}"), "I am not a number");
  // FMT_STRING() above is not needed for c++20

  logd("This message wont be logged since it is lower "
       "than the current log level.");
  fmtlog::setLogLevel(fmtlog::DBG);
  logd("Now debug msg is shown");

  fmtlog::poll();

  fmtlog::setMsgCB(msgCB, nullptr);

  logi("Line breaks: {:s}, {:d}, {:s}", "ABC\n\nXYZ", 123, "D\rE\nFGH");
  logi("Line breaks: {:s}", "ABC XYZ\n");

  fmtlog::poll(true);

  for (int i = 0; i < 3; i++) {
    logio("log once: {}", i);
  }

  fmtlog::setThreadName("main");
  logi("Thread name changed");

  fmtlog::poll();

  fmtlog::setHeaderPattern("{YmdHMSF} {s} {l}[{t}] ");
  logi("Header pattern is changed, full date time info is shown");

  fmtlog::poll();

  for (int i = 0; i < 10; i++) {
    logil(10, "This msg will be logged at an interval of at least 10 ns: {}.", i);
  }

  fmtlog::poll();

  fmtlog::setLogCB(logcb, fmtlog::DBG);
  logw("This msg will be called back");

#ifndef _WIN32
  fmtlog::setLogFile("/tmp/wow", false);
  for (int i = 0; i < 10; i++) {
    logw("test logfilepos: {}.", i);
  }
#endif

  size_t qFullCount = 0;
  fmtlog::setLogQFullCB(logQFullCB, &qFullCount);
  for (int i = 0; i < 1024; i++) {
    std::string str(1000, ' ');
    // log messages are discarded without FMTLOG_BLOCK
    logi("{:d} log q full cb test: {:s}", i, str);
  }

#if FMTLOG_BLOCK==0
  if(qFullCount)
    logi("Discarded {:d} messages", qFullCount);
#endif

  fmtlog::poll();
//  runBenchmark();

  return 0;
}

void runBenchmark() {
  const int RECORDS = 10000;
  // fmtlog::setLogFile("/dev/null", false);
  fmtlog::closeLogFile();
  fmtlog::setLogCB(nullptr, fmtlog::WRN);

  std::chrono::high_resolution_clock::time_point t0, t1;

  t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < RECORDS; ++i) {
    logi("Simple log message with one parameters, {}", i);
  }
  t1 = std::chrono::high_resolution_clock::now();

  double span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count();
  fmt::print("benchmark, front latency: {:.1f} ns/msg average\n", (span / RECORDS) * 1e9);
}
