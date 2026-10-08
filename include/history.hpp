#pragma once

#include <cstddef>
#include <deque>

template <typename T>
class History{
    public:
        explicit History(std::size_t max_size = 60): buffer_size(max_size) {}

        void add(T value){
            history.push_back(value);
            if(history.size() > buffer_size){
                history.pop_front();
            }
        }

        const std::deque<T>& getData() const{
            return history;
        }
    
    private:
        std::deque<T> history;
        std::size_t buffer_size;
};
