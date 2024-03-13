/*
 * Mutex.h
 *
 *  Created on: Feb 29, 2024
 *      Author: Leszek Kosinkiewicz<kosinkiewicz@gmail.com>
 *
 *  this is under the Apache License Version 2.0.
 *
 */
#pragma once
#include "FutexBase.h"
#include "timeManip.h"

namespace futex
{
	class Mutex final : private FutexBase
	{
			static constexpr auto UNLOCK = uint32_t {0};
			static constexpr auto ONE = uint32_t {1};
			static constexpr auto LOCK = ONE;
			static constexpr auto LOCK_WAIT = uint32_t {2};

		public:
			Mutex()
					: FutexBase {UNLOCK}
			{
			}
			Mutex(const Mutex&) = delete;

			inline bool try_lock()
			{
			auto value = UNLOCK;
			return futexVar.compare_exchange_strong(value, LOCK);
			}

			inline void lock()
			{
			if( not try_lock())
				{
					while(futexVar.exchange(LOCK_WAIT) != UNLOCK)
						{
							futex(futexVar, FUTEX_WAIT, LOCK_WAIT);
						};
				}
			}

			inline void unlock()
			{
			if(futexVar.exchange(UNLOCK) == LOCK_WAIT)
				{
					futex(futexVar, FUTEX_WAKE, ONE);
				}
			}

			template<typename clock>
				bool try_lock_until(const std::chrono::time_point<clock> & timeoutPoint)
				{
				while( not try_lock())
					{
						timespec timeout;
						auto now = clock::now();
						while(futexVar.load() != UNLOCK)
							{
								if(now > timeoutPoint)
									{
										return false;
									}
								futexVar.store(LOCK_WAIT);
								timeout = durationToTimespec(timeoutPoint - now);
								futex(futexVar, FUTEX_WAIT, LOCK_WAIT, & timeout);
								now = clock::now();
							}
					}
				return true;
				}

			template<class clock, class Duration>
				bool try_lock_for(const Duration & duration)
				{
				return try_lock_until<clock>(clock::now() + duration);
				}
	};
} //namespace futex
