#pragma once
#include <mutex>
#include <thread>
#include <deque>

template<typename T>
class LockedQueue
{
public:

    LockedQueue( std::deque<T>& queue_in, std::mutex& mutex_in ) :
        lock( mutex_in ),
        queue( queue_in )
    {}

    LockedQueue( const LockedQueue& ) = delete;

    std::deque<T>* operator->() { return &queue; }
private:
    std::unique_lock<std::mutex> lock;
    std::deque<T>& queue;
};

template<typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue( const ThreadSafeQueue& ) = delete;

    LockedQueue<T> Lock() { return LockedQueue<T>( queue, mutex ); }
private:
    std::deque<T> queue;
    std::mutex mutex;
};

template<typename T>
class LockedContainer
{
public:

    LockedContainer( T& container_in, std::mutex& mutex_in ) :
        lock( mutex_in ),
        container( container_in )
    {}

    LockedContainer( const LockedContainer& ) = delete;

    T* operator->() { return &container; }

    T& operator*() { return container; }

private:
    std::unique_lock<std::mutex> lock;
    T& container;
};

template<typename T>
class ThreadSafeContainer
{
public:
    ThreadSafeContainer() = default;
    ThreadSafeContainer( const ThreadSafeContainer& ) = delete;

    LockedContainer<T> Lock() { return LockedContainer<T>( container, mutex ); }
private:
    T container;
    std::mutex mutex;
};
