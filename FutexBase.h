/*
 * FutexBase.h
 *
 *  Created on: Mar 3, 2024
 *      Author: Leszek Kosinkiewicz<kosinkiewicz@gmail.com>
 *
 *  this is under the Apache License Version 2.0.
 *
 */

#pragma once
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <exception>
#include <string>

namespace futex
{
	using Atomic = std::atomic_uint32_t;
	class FutexBase
	{
		protected:
			Atomic futexVar;
			inline FutexBase(const uint32_t value)
					: futexVar {value}
			{
			}

			inline uint32_t get() const
			{
				return futexVar.load();
			}

			inline int futex(Atomic & futexVar, int futex_op, const uint32_t val, struct timespec * timeout = nullptr) const
			{
				const auto ret = syscall(SYS_futex, reinterpret_cast<uint32_t*>( & futexVar), futex_op, val, timeout, nullptr, 0);
				const auto err = errno;
				if ((ret == - 1) and (err != EAGAIN) and (err != ETIMEDOUT))
				{
					constexpr auto SIZE = 256;
					std::array<char, SIZE> buff;
					std::string msg = "futex error: " + std::to_string(err) + " @ " + strerror_r(err, buff.data(), SIZE);
					throw std::runtime_error(msg);
				}
				return ret;
			}
	};
} //namespace futex
