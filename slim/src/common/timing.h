// MIT License
//
// Copyright (c) 2019 Daniel Kocher
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef COMMON_TIMING_H
#define COMMON_TIMING_H

#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include <climits>
#include <vector>
#include <string>

namespace common {

struct Interval {
#ifdef GETRUSAGE
	struct timeval usertime;
	struct timeval usertime_start;
	struct timeval systemtime;
	struct timeval systemtime_start;
#else
	struct timespec time;
	struct timespec time_start;
#endif

	Interval();
	double getfloat() const;
	void start();
	void stop();
	void reset() noexcept;
};

struct TimingEmpty {
	static inline void start(Interval& interval) {}
	static inline void stop(Interval& interval) {}
	static inline void reset(Interval& interval) {}
	static inline void setvalue(const Interval& interval, double& val) {}
	static inline void addvalue(const Interval& interval, double& val) {}
	static inline void resetstart(Interval& interval) {}
	static inline void stopsetvalue(const Interval& interval, double& val) {}
	static inline void stopaddvalue(const Interval& interval, double& val) {}
};

struct Timing {
	static inline void start(Interval& interval) {
		interval.start();
	}

	static inline void stop(Interval& interval) {
		interval.stop();
	}

	static inline void reset(Interval& interval) {
		interval.reset();
	}

	static inline void setvalue(const Interval& interval, double& val) {
		val = interval.getfloat();
	}

	static inline void addvalue(const Interval& interval, double& val) {
		val += interval.getfloat();
	}

	static inline void resetstart(Interval& interval) {
		reset(interval);
		start(interval);
	}

	static inline void stopsetvalue(Interval& interval, double& val) {
		stop(interval);
		setvalue(interval, val);
	}

	static inline void stopaddvalue(Interval& interval, double& val) {
		stop(interval);
		addvalue(interval, val);
	}
};

#ifdef NO_INALGO_TIMINGS
struct InAlgoTiming : public TimingEmpty {};
#else
struct InAlgoTiming : public Timing {};
#endif // NO_INALGO_TIMINGS

#ifdef NO_WRAPPING_TIMINGS
struct WrappingTiming : public TimingEmpty {};
#else
struct WrappingTiming : public Timing {};
#endif // NO_WRAPPING_TIMINGS

} // namespace common

#endif // COMMON_TIMING_H
