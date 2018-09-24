#ifndef __TEST_MESSAGING_HPP__
#define __TEST_MESSAGING_HPP__


#include "CProxyLogger.hpp"

#include "catch.hpp"

#include <list>
#include <thread>
#include <algorithm>
#include <chrono>

using namespace std;

static std::list<std::string> Errors;

class CLoggerTest : public cf::ILogError
{
public:
    void log(const std::string& message) {
        Errors.push_back(message);
    }
    
};


static cf::CProxyLogger ProxyLogger{ std::make_unique< CLoggerTest >() };


TEST_CASE("MESSAGING")
{
    constexpr int NB_MESSAGES = 10000;
    auto messages_thread_1 = make_shared<list<std::string>>();
    auto messages_thread_2 = make_shared<list<std::string>>();
    auto messages_thread_3 = make_shared<list<std::string>>();

    const auto randchar = []() -> char
    {
        constexpr char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        constexpr size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };

    const auto randstr = [randchar]() -> string
    {
        const size_t length = rand() % 64;
        string str(length, 0);
        generate_n(str.begin(), length, randchar);
        return str;
    };

    auto task_1 = thread([&NB_MESSAGES, messages_thread_1, randstr]()
    {
        using namespace std::chrono_literals;
        this_thread::sleep_for(100ms);
        for (auto i = 0; i < NB_MESSAGES; ++i)
        {
            const auto str = randstr();
            messages_thread_1->push_back(str);
            ProxyLogger.error(str);            
        }
    });

    auto task_2 = thread([&NB_MESSAGES, messages_thread_2, randstr]()
    {
        using namespace std::chrono_literals;
        this_thread::sleep_for(100ms);
        for (auto i = 0; i < NB_MESSAGES; ++i)
        {
            const auto str = randstr();
            messages_thread_2->push_back(str);
            ProxyLogger.error(str);
        }
    });

    auto task_3 = thread([&NB_MESSAGES, messages_thread_3, randstr]()
    {
        using namespace std::chrono_literals;
        this_thread::sleep_for(100ms);
        for (auto i = 0; i < NB_MESSAGES; ++i)
        {
            const auto str = randstr();
            messages_thread_3->push_back(str);
            ProxyLogger.error(str);
        }
    });

    task_1.join();
    task_2.join();
    task_3.join();

    this_thread::sleep_for(1000ms);

    REQUIRE(Errors.size() == 3 * NB_MESSAGES);
    
    auto all_messages_thread1_present = true;
    for (const auto message : *messages_thread_1) {
        if (std::find(begin(Errors), end(Errors), message) == end(Errors)) {            
            all_messages_thread1_present = false;
            break;
        }
    }
    REQUIRE(all_messages_thread1_present);

    auto all_messages_thread2_present = true;
    for (const auto message : *messages_thread_2) {
        if (std::find(begin(Errors), end(Errors), message) == end(Errors)) {
            all_messages_thread2_present = false;
            break;
        }
    }
    REQUIRE(all_messages_thread2_present);

    auto all_messages_thread3_present = true;
    for (const auto message : *messages_thread_3) {
        if (std::find(begin(Errors), end(Errors), message) == end(Errors)) {
            all_messages_thread1_present = false;
            break;
        }
    }
    REQUIRE(all_messages_thread3_present);
}


#endif
