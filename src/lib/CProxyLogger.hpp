#ifndef _SRC_LOGGERCONCURRENT_HPP__
#define _SRC_LOGGERCONCURRENT_HPP__

#include "CompareFolders.hpp"
#include "TDequeConcurrent.hpp"

#include <string>
#include <thread>

namespace cf {

    /// @brief This class accepts Messages and Error from threads threads and forwards them safely to the concrete logger
    class CProxyLogger
    {
    public:
        /// @details Takes exclusive ownership of loggers
        CProxyLogger(std::unique_ptr<ILogger> loggers) :
            _ptr_loggers{ std::move(loggers) }
          , _stopRequested{ false }
        {
            //_task_messages.detach();
            //_task_errors.detach();
        }
        ~CProxyLogger() = default;

        /// @brief Sends a message
        inline void message(const std::string& msg) {
            _queue_messages.emplace_back(msg);
        }
        /// @brief Sends an error
        inline void error(const std::string& err) {
            _queue_errors.emplace_back(err);
        }

        inline void stop() {
          _stopRequested.store(true, std::memory_order_release);
          _queue_messages.stop();
          _queue_errors.stop();
          _task_messages.join();          
          _task_errors.join();
        }

    private:
        TDequeConcurrent<std::string> _queue_messages;  ///< thread safe dequeue used to pile messages to be sent
        TDequeConcurrent<std::string> _queue_errors;    ///< thread safe dequeue used to pile  errors to be sent
        std::unique_ptr<ILogger> _ptr_loggers;
        std::mutex _mutex_logger;                       ///< the logger may be called from two threads: it needs to be guarded
        std::atomic_bool _stopRequested;

        /// @brief Messages are logged in this thread, to avoid blocking main
        std::thread _task_messages = std::thread{ [this]
        {
            while (!_stopRequested.load(std::memory_order_relaxed)) {
                auto message = this->_queue_messages.pop_front(); //blocks until a message is in the list
                {
                  if (!message.empty()) {
                    const std::lock_guard<std::mutex> lock{ this->_mutex_logger };
                    _ptr_loggers->message(message);
                  }
                }
            }
        } };
        /// @brief Errors are logged in this thread, to avoid blocking main
        std::thread _task_errors = std::thread{ [this]
        {
            while (!_stopRequested.load(std::memory_order_relaxed)) {
                auto message = this->_queue_errors.pop_front(); //blocks until a message is in the list
                {
                  if (!message.empty()) {
                    const std::lock_guard<std::mutex> lock{ this->_mutex_logger };
                    _ptr_loggers->error(message);
                  }
                }
            }
        } };
    };

}

#endif
