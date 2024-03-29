/* 
 *  Copyright (C) Christophe Meneboeuf <christophe@xtof.info>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef CONCURRENT_DEQUE_H
#define CONCURRENT_DEQUE_H

#include <deque>
#include <mutex>
#include <atomic>
#include <condition_variable>


namespace cf {

    //! @brief A templated *thread-safe* collection based on dequeue
    //!
    //! @details    pop_front() waits for the notification of a filling method if the collection is empty.
    //!             The various "emplace" operations are factorized by using the generic "addData_protected".
    //!             This generic asks for a concrete operation to use, which can be passed as a lambda.
    template< typename T >
    class TDequeConcurrent {

    public:
      TDequeConcurrent() :
        _stopRequested{false}
      {      }

        //! \brief Emplaces a new instance of T in front of the deque
        template<typename... Args>
        void emplace_front(Args&&... args)
        {
            addData_protected([&] {
                _collection.emplace_front(std::forward<Args>(args)...);
            });
        }

        //! \brief Emplaces a new instance of T at the back of the deque
        template<typename... Args>
        void emplace_back(Args&&... args)
        {
            addData_protected([&] {
                _collection.emplace_back(std::forward<Args>(args)...);
            });
        }

        //! \brief Clears the deque
        void clear(void)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _collection.clear();
        }

        //! \brief Returns the front element and removes it from the collection
        //!
        //!        No exception is ever returned as we garanty that the deque is not empty
        //!        before trying to return data.
        T pop_front(void) noexcept
        {
          T elem;

          if (!_stopRequested.load(std::memory_order_relaxed))
          {
            std::unique_lock<std::mutex> lock{ _mutex };
            while (_collection.empty()) {
              _condNewData.wait(lock);
              if (_stopRequested.load(std::memory_order_relaxed)) {
                break;
              }
            }

            if (!_stopRequested.load(std::memory_order_relaxed)) {
              elem = std::move(_collection.front());
              _collection.pop_front();
            }
          }

          return elem;
        }

        //! \brief returns true if the collection is empty
        bool empty() const
        {
          std::lock_guard<std::mutex> lock{ _mutex };
          return _collection.empty();
        }

        //! \brief stops waiting for new messages
        void stop()
        {
          _stopRequested.store(true, std::memory_order_release);
          _condNewData.notify_all();
        }



    private:

        //! \brief Protects the deque, calls the provided function and notifies the presence of new data
        //! \param The concrete operation to be used. It MUST be an operation which will add data to the deque,
        //!        as it will notify that new data are available!
        template<class F>
        void addData_protected(F&& fct)
        {
          {
            std::lock_guard<std::mutex> lock{ _mutex };
            fct();
          }
            _condNewData.notify_one();
        }

        std::deque<T> _collection;              ///< Concrete, not thread safe, storage.
        mutable std::mutex    _mutex;                   ///< Mutex protecting the concrete storage
        std::condition_variable _condNewData;   ///< Condition used to notify that new data are available.

        std::atomic_bool _stopRequested;
    };

}

#endif // CONCURRENT_DEQUE_H
