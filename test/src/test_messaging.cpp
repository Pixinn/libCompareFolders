#ifndef __TEST_MESSAGING_HPP__
#define __TEST_MESSAGING_HPP__


#include "CProxyLogger.hpp"

#include "catch.hpp"

#include <list>
#include <set>
#include <thread>
#include <algorithm>
#include <chrono>

using namespace std;

static std::set<std::string> Errors;
static std::set<std::string> Messages;

class CLoggerTest : public cf::ILogger
{
public:
    void error(const std::string& message) {
        Errors.insert(message);
    }
    void message(const std::string& message) {
        Messages.insert(message);
    }

};


static cf::CProxyLogger ProxyLogger{ std::make_unique< CLoggerTest >() };


TEST_CASE("MESSAGING")
{
    static constexpr int NB_MESSAGES = 10000;
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
        const size_t length = rand() % 32;
        string str(length, 0);
        generate_n(str.begin(), length, randchar);
        return str;
    };

    auto task_1 = thread([messages_thread_1, randstr]()
    {
        using namespace std::chrono_literals;
        this_thread::sleep_for(100ms);
        for (auto i = 0; i < NB_MESSAGES; ++i)
        {
            const auto str = std::string("1-") + std::to_string(i) + std::string("-") + randstr();
            messages_thread_1->push_back(str);
            ProxyLogger.message(str);           
            ProxyLogger.error(str);
        }
    });

    auto task_2 = thread([messages_thread_2, randstr]()
    {
        using namespace std::chrono_literals;
        this_thread::sleep_for(120ms);
        for (auto i = 0; i < NB_MESSAGES; ++i)
        {
          const auto str = std::string("2-") + std::to_string(i) + std::string("-") + randstr();
            messages_thread_2->push_back(str);
            ProxyLogger.error(str);
            ProxyLogger.message(str);
        }
    });

    auto task_3 = thread([messages_thread_3, randstr]()
    {
        using namespace std::chrono_literals;
        this_thread::sleep_for(140ms);
        for (auto i = 0; i < NB_MESSAGES; ++i)
        {
           const auto str = std::string("3-") + std::to_string(i) + std::string("-") + randstr();
            messages_thread_3->push_back(str);
            ProxyLogger.message(str);
            ProxyLogger.error(str);
        }
    });

    task_1.join();
    task_2.join();
    task_3.join();

    this_thread::sleep_for(1000ms);
    ProxyLogger.stop();

    const std::size_t nbMessagesSents = 
      messages_thread_1->size() + messages_thread_2->size() + messages_thread_3->size();
    REQUIRE(Errors.size() == nbMessagesSents);
    REQUIRE(Messages.size() == nbMessagesSents);

    const auto same_messages = (Errors == Messages);
    REQUIRE(same_messages);
    
    auto all_messages_thread1_present = true;
    for (const auto& message : *messages_thread_1) {
        if (std::find(begin(Errors), end(Errors), message) == end(Errors)) {            
            all_messages_thread1_present = false;
            break;
        }
    }
    REQUIRE(all_messages_thread1_present);

    auto all_messages_thread2_present = true;
    for (const auto& message : *messages_thread_2) {
        if (std::find(begin(Errors), end(Errors), message) == end(Errors)) {
            all_messages_thread2_present = false;
            break;
        }
    }
    REQUIRE(all_messages_thread2_present);

    auto all_messages_thread3_present = true;
    for (const auto& message : *messages_thread_3) {
        if (std::find(begin(Errors), end(Errors), message) == end(Errors)) {
            all_messages_thread1_present = false;
            break;
        }
    }
    REQUIRE(all_messages_thread3_present);
}


#endif
