/*
 * timeManip.h
 *
 *  Created on: Mar 6, 2024
 *      Author: Leszek Kosinkiewicz
 */

#pragma once
#include <chrono>
namespace
{
template< class Duration >
timespec durationToTimespec(const Duration dur)
{
   const auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
   return {secs.count(), std::chrono::duration_cast<std::chrono::nanoseconds>(dur - secs).count()};
}
} //namespace
