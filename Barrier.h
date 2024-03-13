/*
 * Barrier.h
 *
 *  Created on: Feb 29, 2024
 *      Author: Leszek Kosinkiewicz<kosinkiewicz@gmail.com>
 *
 *  this is under the Apache License Version 2.0.
 *
 */
#pragma once
#include <atomic>
#include "FutexBase.h"
#include "timeManip.h"

namespace futex
{
	class Barrier final : private FutexBase
	{
			static constexpr auto ZERO = uint32_t {0};
			static constexpr auto ONE = uint32_t {1};

		public:
			Barrier(const uint32_t count)
					: FutexBase {count}, count {count - ONE}
			{
			}
			Barrier(const Barrier&) = delete;

			using FutexBase::get;

			inline void wait()
			{
			if(futexVar.fetch_sub(ONE) == ONE)
				{
					futex(futexVar, FUTEX_WAKE, count);
					return;
				}

			uint32_t value;
			while((value = futexVar.load()) != ZERO)
				{
					futex(futexVar, FUTEX_WAIT, value);
				}
			}

			template<typename clock>
				bool wait_until(const std::chrono::time_point<clock> & timeoutPoint)
				{
				if(futexVar.fetch_sub(ONE) == ONE)
					{
						futex(futexVar, FUTEX_WAKE, count);
						return true;
					}

				uint32_t value;
				timespec timeout;
				auto now = clock::now();
				while((value = futexVar.load()) != ZERO)
					{
						if(now > timeoutPoint)
							{
								return false;
							}
						timeout = durationToTimespec(timeoutPoint - now);
						futex(futexVar, FUTEX_WAIT, value, & timeout);
						now = clock::now();
					}
				return true;
				}

			template<class clock, class Duration>
				bool wait_for(const Duration & duration)
				{
				return wait_until<clock>(clock::now() + duration);
				}

			inline int signal()
			{
			futexVar.exchange(ZERO);
			return futex(futexVar, FUTEX_WAKE, INT32_MAX);
			}

		private:
			const uint32_t count;
	};
} // namespace futex
