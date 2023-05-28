#ifndef _SRC_LOGGERCONCURRENT_HPP__
#define _SRC_LOGGERCONCURRENT_HPP__

#include "CompareFolders.hpp"
#include "TDequeConcurrent.hpp"
#include <iostream>
#include <string>
#include <thread>

namespace cf {

    /// @brief This class accepts Messages and Error from threads and forwards them safely to the concrete logger
    class CProxyLogger
    {
    public:
        /// @details Takes exclusive ownership of loggers
        CProxyLogger(std::unique_ptr<ILogger> loggers) :
            _ptr_loggers{ std::move(loggers) }
        {
            startThreads();
        }
        ~CProxyLogger() {
           stopThreads();
        }

        /// @brief Sends a message
        inline void message(const std::string& msg) {
            _queue_messages.emplace_back(msg);
        }
        /// @brief Sends an error
        inline void error(const std::string& err) {
            _queue_errors.emplace_back(err);
        }

    private:
        TDequeConcurrent<std::string> _queue_messages;  ///< thread safe dequeue used to pile messages to be sent
        TDequeConcurrent<std::string> _queue_errors;    ///< thread safe dequeue used to pile  errors to be sent
        std::unique_ptr<ILogger> _ptr_loggers;
        std::mutex _mutex_logger;                       ///< the logger may be called from two threads: it needs to be guarded
        bool _threadsAreUp;
        std::thread _task_messages;
        std::thread _task_errors;

        /// @details Checks if the state of the message and error threads is up or down.
        ///          Thread state is common to both threads as they are started and shut down at the same time.
        inline bool checkThreadsAreUp(){
            std::lock_guard<std::mutex>{this->_mutex_logger};
            return _threadsAreUp ;
        }

        /// @brief Starts message and error threads
        inline void startThreads(){
            {
                std::lock_guard<std::mutex>{this->_mutex_logger};
                _threadsAreUp = true ;
            }
            launchMessageThread();
            launchErrorThread();
            _task_messages.detach();
            _task_errors.detach();

        }

        /// @brief Stops message and error threads
        inline void stopThreads(){
            {
                std::lock_guard<std::mutex>{this->_mutex_logger};
                _threadsAreUp = false ;
            }
        }

        inline void launchMessageThread(){
        /// @brief Messages are logged in this thread, to avoid blocking main
         _task_messages = std::thread{ [this]
        {
            while (checkThreadsAreUp()) {
               if (! _queue_messages.empty()){
                 auto message = this->_queue_messages.pop_front();
                    {
                        std::lock_guard<std::mutex>{this->_mutex_logger};
                        _ptr_loggers->message(message);
                    }
                }
            }
        } };
        }

        inline void launchErrorThread(){
        /// @brief Errors are logged in this thread, to avoid blocking main
        _task_errors = std::thread{ [this]
        {
            while (checkThreadsAreUp()) {
                if (! _queue_errors.empty()){
                    auto message = this->_queue_errors.pop_front();
                    {
                        std::lock_guard<std::mutex>{this->_mutex_logger};
                        _ptr_loggers->error(message);
                    }
                }
            }
        } };
        }
    };

}

#endif
