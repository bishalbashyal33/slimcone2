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

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <cassert>

#include "timing.h"

namespace common {

struct timespec operator-(const struct timespec& ts1, const struct timespec& ts2)
{
	struct timespec res;

	if (ts1.tv_sec >= 0 && ts2.tv_sec >= 0) {
		res.tv_sec = ts1.tv_sec - ts2.tv_sec;
		res.tv_nsec = ts1.tv_nsec - ts2.tv_nsec;

		if (res.tv_nsec < 0) {
			res.tv_nsec += 1000000000;
			res.tv_sec -= 1;
		}
	} else {
		std::cerr << "not implemented";
		abort();
	}

	return res;
}

struct timespec operator+(const struct timespec& ts1, const struct timespec& ts2)
{
	struct timespec res;

	if (ts1.tv_sec >= 0 && ts2.tv_sec >= 0) {
		res.tv_sec = ts1.tv_sec + ts2.tv_sec;
		res.tv_nsec = ts1.tv_nsec + ts2.tv_nsec;

		if (res.tv_nsec > 1000000000) {
			res.tv_nsec -= 1000000000;
			res.tv_sec += 1;
		}
	} else {
		std::cerr << "not implemented";
		abort();
	}

	return res;
}

std::ostream& operator<<(std::ostream& os, const struct timespec& ts) {
	char prev;

	os << ts.tv_sec << ".";
	prev = os.fill('0');
	os << std::setw(9) << ts.tv_nsec;
	os.fill(prev);

	return os;
}

void cleantimespec(struct timespec & ts) {
	ts.tv_sec = ts.tv_nsec = 0;
}

Interval::Interval() {
	reset();
}

double Interval::getfloat() const {
#ifdef GETRUSAGE
    return usertime.tv_usec / 1000.0 + usertime.tv_sec * 1000.0 +
            systemtime.tv_usec / 1000.0 + systemtime.tv_sec * 1000.0;
#else
    return time.tv_nsec / 1000000.0 + time.tv_sec * 1000.0;
#endif
}

void Interval::start() {
#ifdef GETRUSAGE
	struct rusage ru;
	int ret = getrusage(RUSAGE_SELF, &ru);
	if (ret != 0) {
		perror("Timing call to getrusage");
		abort();
	}

	usertime_start = ru.ru_utime;
	systemtime_start = ru.ru_stime;
#else
	int ret = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
	if (ret != 0) {
		perror("Timing call to getrusage");
		abort();
	}
#endif
}

void Interval::stop() {
#ifdef GETRUSAGE
	struct rusage ru;
	int ret = getrusage(RUSAGE_SELF, &ru);
	if (ret != 0) {
		perror("Timing call to getrusage");
		abort();
	}
#else
	struct timespec tsnow;
	int ret = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tsnow);
	if (ret != 0) {
		perror("Timing call to getrusage");
		abort();
	}
#endif

#ifdef GETRUSAGE
	struct timeval systemres, userres, tmp;

	timersub(&ru.ru_utime, &usertime_start, &userres);
	timersub(&ru.ru_stime, &systemtime_start, &systemres);

	timeradd(&usertime, &userres, &tmp);
	usertime = tmp;

	timeradd(&systemtime, &systemres, &tmp);
	systemtime = tmp;

#else
	time = tsnow - time_start + time;
#endif
}

void Interval::reset() noexcept {
#ifdef GETRUSAGE
	timerclear(&usertime);
	timerclear(&systemtime);
#else
	cleantimespec(time);
#endif
}



} // namespace common
