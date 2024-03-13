/*
 * test.cpp
 *
 *  Created on: Feb 29, 2024
 *      Author: Leszek Kosinkiewicz<kosinkiewicz@gmail.com>
 *
 *  this is under the Apache License Version 2.0.
 *
 */

#include <gtest/gtest.h>

#include <array>
#include <thread>
#include <mutex>
#include <list>

#include "Barrier.h"
#include "BarrierExt.h"
#include "Mutex.h"

using namespace std::literals;
using namespace testing;

constexpr auto NUMBER = 10'000'000;
template<typename V = int64_t, typename T = std::mutex>
	void suma(V & n, T * mutex = nullptr)
	{
	for(int i = 0; i < NUMBER; ++ i)
		{
			if(mutex != nullptr)
				{
					mutex->lock();
				}
			n += 1;
			if(mutex != nullptr)
				{
					mutex->unlock();
				}
		}
	}

TEST(SUMA, DISABLED_NO_PROTECT)
{
int64_t v = 0;
std::thread t1(suma<>, std::ref(v), nullptr);
std::thread t2(suma<>, std::ref(v), nullptr);
std::thread t3(suma<>, std::ref(v), nullptr);
t1.join();
t2.join();
t3.join();
EXPECT_NE(v, 3 * NUMBER);
}

TEST(SUMA, DISABLED_ATOMIC_PROTECT)
{
std::atomic_int64_t v {0};
std::thread t1(suma<std::atomic_int64_t>, std::ref(v), nullptr);
std::thread t2(suma<std::atomic_int64_t>, std::ref(v), nullptr);
std::thread t3(suma<std::atomic_int64_t>, std::ref(v), nullptr);
t1.join();
t2.join();
t3.join();
EXPECT_EQ(v, 3 * NUMBER);
}

TEST(SUMA, DISABLED_STD_MUTEX_PROTECT)
{
int64_t v = 0;
std::mutex mutex;
std::thread t1(suma<>, std::ref(v), & mutex);
std::thread t2(suma<>, std::ref(v), & mutex);
std::thread t3(suma<>, std::ref(v), & mutex);
t1.join();
t2.join();
t3.join();
EXPECT_EQ(v, 3 * NUMBER);
}

TEST(SUMA, FUTEX_MUTEX_PROTECT)
{
int64_t v = 0;
futex::Mutex mutex;
std::thread t1(suma<int64_t, futex::Mutex>, std::ref(v), & mutex);
std::thread t2(suma<int64_t, futex::Mutex>, std::ref(v), & mutex);
std::thread t3(suma<int64_t, futex::Mutex>, std::ref(v), & mutex);
t1.join();
t2.join();
t3.join();
EXPECT_EQ(v, 3 * NUMBER);
}

TEST(FUTEX, BARRIER_WAIT)
{
constexpr auto SIZE = 30;
std::array<std::thread, SIZE> tabThread;
futex::Barrier barrier(SIZE + 1); // SIZE threads + 1 main
std::list<std::thread::id> ids {};
futex::Mutex mutex;

auto lambda = [&barrier, &ids, &mutex]()
	{
			{
				std::lock_guard<futex::Mutex> guard(mutex);
				ids.push_back(std::this_thread::get_id());
			}
		barrier.wait();
	};
for(auto && t : tabThread)
	{
		t = std::thread {lambda};
	}
barrier.wait();
EXPECT_EQ(ids.size(), SIZE);
for(auto && t : tabThread)
	{
		t.join();
	}
}

TEST(FUTEX, BARRIER_WAIT_FOR_TIMEOUT)
{
futex::Barrier barrierExt {2};
auto t1 = std::chrono::steady_clock::now();
EXPECT_FALSE(barrierExt.wait_for<std::chrono::steady_clock>(1ms));
auto t2 = std::chrono::steady_clock::now();
EXPECT_GT(t2 - t1, 1ms);
}

TEST(FUTEX, BARRIER_WAIT_FOR_NO_TIMEOUT)
{
constexpr auto SIZE = 30;
std::array<std::thread, SIZE> tabThread;
futex::Barrier barrier(SIZE + 1); // SIZE threads + 1 main
std::list<std::thread::id> ids {};
futex::Mutex mutex;

auto lambda = [&barrier, &ids, &mutex]()
	{
			{
				std::lock_guard<futex::Mutex> guard(mutex);
				ids.push_back(std::this_thread::get_id());
			}
		EXPECT_TRUE(barrier.wait_for<std::chrono::steady_clock>(1s));
	};

for(auto && t : tabThread)
	{
		t = std::thread {lambda};
	}

EXPECT_TRUE(barrier.wait_for<std::chrono::steady_clock>(1s));
EXPECT_EQ(ids.size(), SIZE);

for(auto && t : tabThread)
	{
		t.join();
	}
}

TEST(FUTEX, BARRIER_WAIT_SIGNAL)
{
constexpr auto SIZE = 30;
std::array<std::thread, SIZE> tabThread;
futex::Barrier barrier(SIZE + 1); // SIZE threads + 1 main
std::list<std::thread::id> ids {};
futex::Mutex mutex;

auto lambda = [&barrier, &ids, &mutex]()
	{
			{
				std::lock_guard<futex::Mutex> guard(mutex);
				ids.push_back(std::this_thread::get_id());
			}
		barrier.wait();
	};

for(auto && t : tabThread)
	{
		t = std::thread {lambda};
	}

while(barrier.get() != 1)
	{
		std::this_thread::sleep_for(200us);
	}

barrier.signal();
EXPECT_EQ(ids.size(), SIZE);

for(auto && t : tabThread)
	{
		t.join();
	}
}

TEST(FUTEX, BARRIER_WAIT_FOR_NO_TIMEOUT_SIGNAL)
{
constexpr auto SIZE = 30;
std::array<std::thread, SIZE> tabThread;
futex::Barrier barrier(SIZE + 1); // SIZE threads + 1 main
std::list<std::thread::id> ids {};
futex::Mutex mutex;

auto lambda = [&barrier, &ids, &mutex]()
	{
			{
				std::lock_guard<futex::Mutex> guard(mutex);
				ids.push_back(std::this_thread::get_id());
			}
		EXPECT_TRUE(barrier.wait_for<std::chrono::steady_clock>(1s));
	};

for(auto && t : tabThread)
	{
		t = std::thread {lambda};
	}

while(barrier.get() != 1)
	{
		std::this_thread::sleep_for(200us);
	}

barrier.signal();

EXPECT_EQ(ids.size(), SIZE);
for(auto && t : tabThread)
	{
		t.join();
	}
}

TEST(FUTEX, TRY_LOCK)
{
futex::Mutex mutex;
futex::Barrier barrier {2};
futex::Barrier barrierTwo {2};

auto lambda = [&mutex, &barrier, &barrierTwo]()
	{
		mutex.lock();
		barrier.wait();

		barrierTwo.wait();
		mutex.unlock();
	};

std::thread t(lambda);
barrier.wait();
EXPECT_EQ(mutex.try_lock(), false);
barrierTwo.wait();
t.join();
EXPECT_EQ(mutex.try_lock(), true);
mutex.unlock();
}

TEST(FUTEX, LOCK_ON_THE_SAME_THREAD_HUNG_UP_UNLOCK_FORM_OUTSIDE)
{
futex::Mutex mutex;
futex::Barrier barrier1 {2};
futex::Barrier barrier2 {2};
futex::Barrier barrier3 {2};
futex::Barrier barrier4 {2};
std::atomic_int lockLevel {0};

auto lambda = [&mutex, &barrier1, &barrier2, &barrier3, &barrier4, &lockLevel]()
	{
		mutex.lock();
		++lockLevel; //1
		barrier1.wait();

		mutex.lock();
		++lockLevel;//2
		barrier2.wait();

		mutex.lock();
		++lockLevel;//3
		barrier3.wait();

		mutex.lock();
		++lockLevel;//4
		barrier4.wait();

		mutex.unlock();
	};

std::thread t(lambda);
auto i = 0;
barrier1.wait();
EXPECT_EQ(lockLevel.load(), ++ i); //1
mutex.unlock(); // unlock the second lock

barrier2.wait();
EXPECT_EQ(lockLevel.load(), ++ i); //2
mutex.unlock(); // unlock the third lock

barrier3.wait();
EXPECT_EQ(lockLevel.load(), ++ i); //3
mutex.unlock(); // unlock the fourth lock

barrier4.wait();
EXPECT_EQ(lockLevel.load(), ++ i); //4
t.join();
EXPECT_EQ(mutex.try_lock(), true);
}

TEST(FUTEX, TRY_LOCK_FOR)
{
futex::Mutex mutex;
futex::Barrier barrier {2};
auto lambda = [&mutex, &barrier]()
	{
		mutex.lock();
		barrier.wait();
		std::this_thread::sleep_for(5ms);
		mutex.unlock();
	};

std::thread t(lambda);
barrier.wait();

EXPECT_EQ(mutex.try_lock(), false);
EXPECT_EQ(mutex.try_lock_for<std::chrono::steady_clock>(3ms), false);
EXPECT_EQ(mutex.try_lock_for<std::chrono::steady_clock>(5ms), true);
t.join();
EXPECT_EQ(mutex.try_lock(), false);
}

TEST(FUTEX, BARRIER_EXT_WAIT_SIGNAL)
{
constexpr auto SIZE = 30;
futex::BarrierExt barrierExt;
std::list<std::thread::id> ids {};
futex::Mutex mutex;

auto lambda = [&barrierExt, &ids, &mutex]()
	{
		auto id = std::this_thread::get_id();
			{
				std::lock_guard<futex::Mutex> guard(mutex);
				ids.push_back(id);
			}
		barrierExt.wait();
	};

std::array<std::thread, SIZE> tab;
for(auto && t : tab)
	{
		t = std::thread {lambda};
	}

while(barrierExt.get() != SIZE)
	{
		std::this_thread::sleep_for(200us);
	}

barrierExt.signal();
EXPECT_EQ(ids.size(), SIZE);
for(auto && t : tab)
	{
		t.join();
	}
}

TEST(FUTEX, BARRIER_EXT_WAIT_FOR_NO_TIMEOUT_SIGNAL)
{
constexpr auto SIZE = 30;
futex::BarrierExt barrierExt;
std::list<std::thread::id> ids {};
futex::Mutex mutex;

auto lambda = [&barrierExt, &ids, &mutex]()
	{
		auto id = std::this_thread::get_id();
			{
				std::lock_guard<futex::Mutex> guard(mutex);
				ids.push_back(id);
			}
		barrierExt.wait_for<std::chrono::steady_clock>(1s);
	};

std::array<std::thread, SIZE> tab;
for(auto && t : tab)
	{
		t = std::thread {lambda};
	}

while(barrierExt.get() != SIZE)
	{
		std::this_thread::sleep_for(200us);
	}

barrierExt.signal();
EXPECT_EQ(ids.size(), SIZE);
for(auto && t : tab)
	{
		t.join();
	}
}

TEST(FUTEX, BARRIER_EXT_WAIT_FOR_TIMEOUT)
{
futex::BarrierExt barrierExt;
auto t1 = std::chrono::steady_clock::now();
EXPECT_FALSE(barrierExt.wait_for<std::chrono::steady_clock>(1ms));
auto t2 = std::chrono::steady_clock::now();
EXPECT_GT(t2 - t1, 1ms);
}
