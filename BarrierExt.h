/*
 * BarrierExt.h
 *
 *  Created on: Mar 2, 2024
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
	class BarrierExt final : private FutexBase
	{
			static constexpr auto ZERO = uint32_t {0};
			static constexpr auto ONE = uint32_t {1};

		public:
			BarrierExt()
					: FutexBase {ZERO}
			{
			}
			BarrierExt(const BarrierExt&) = delete;

			using FutexBase::get;

			inline void wait()
			{
				futexVar.fetch_add(ONE);
				uint32_t value;
				while((value = futexVar.load()) != ZERO)
				{
					futex(futexVar, FUTEX_WAIT, value);
				}
			}

			template<typename clock>
				bool wait_until(const std::chrono::time_point<clock> & timeoutPoint)
				{
					futexVar.fetch_add(ONE);
					uint32_t value;
					timespec timeout;
					auto now = clock::now();
					while((value = futexVar.load()) != ZERO)
					{
						if (now > timeoutPoint)
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
	};
} // namespace futex

