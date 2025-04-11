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
