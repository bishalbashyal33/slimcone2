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

#ifndef COMMON_STATISTICS_H
#define COMMON_STATISTICS_H

#include <climits>
#include <ostream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include <map>
#include <vector>

namespace common {

#ifdef NO_STAT_COUNTERS
struct RealUpdater {
	template<class T> inline static void inc(T&) {}
	template<class T> inline static void add(T&, T) {}
	template<class T> inline static void set(T&, T) {}
};

struct MapUpdater {
	template<class M> inline static void inc(M&, typename M::key_type&) {}
	template<class M> inline static void add(
			M&,
			typename M::key_type&,
			typename M::mapped_type&) {}
	template<class M> inline static void set(
			M&,
			typename M::key_type&,
			typename M::mapped_type&) {}
	template<class M> inline static void clear(M&) {}
};
#else
struct RealUpdater {
	template<class T> inline static void inc(T& val) {
		val += 1;
	}

	template<class T> inline static void add(T& val, T a) {
		val += a;
	}

	template<class T> inline static void set(T& val, T a) {
		val = a;
	}
};

struct MapUpdater {
	template<class M> inline static void inc(M& map, typename M::key_type& pos) {
		map[pos] += 1;
	}

	template<class M> inline static void add(
			M& map,
			typename M::key_type& pos,
			typename M::mapped_type& a) {
		map[pos] += a;
	}

	template<class M> inline static void set(
			M& map,
			typename M::key_type& pos,
			typename M::mapped_type& a) {
		map[pos] = a;
	}

	template<class M> inline static void clear(M& map) {
		map.clear();
	}
};
#endif

template<
	typename SingleUpdater = RealUpdater,
	typename HistUpdater = MapUpdater>
class Statistics {
public:
  // StatItem: Just a counter
	struct StatItem {
		unsigned long value;

    StatItem() : value(0) {}

    inline void inc() {
			SingleUpdater::inc(value);
		}

		inline void add(unsigned long a) {
			SingleUpdater::add(value, a);
		}

		inline void set(unsigned long a) {
			SingleUpdater::set(value, a);
	  }

		inline void reset() {
			set(0);
		}
	};

  // StatFloatItem: Just a floating point representing, e.g., timing
  struct StatFloatItem {
    double value;

    StatFloatItem() : value(0.0) {}

    inline void add(double a) {
      SingleUpdater::add(value, a);
    }

    inline void set(double a) {
      SingleUpdater::set(value, a);
    }

    inline void reset() {
      SingleUpdater::reset(0.0);
    }
  };

  // StatAvgItem: report integer values, compute avg, min, max
	struct StatAvgItem {
		unsigned long sum;
		unsigned long count;
		unsigned long min;
		unsigned long max;

    StatAvgItem() : sum(0UL), count(0UL), min(ULONG_MAX), max(0UL) {}

    inline void record(unsigned long a) {
			SingleUpdater::inc(count);
			SingleUpdater::add(sum, a);
			SingleUpdater::set(min, std::min(min, a));
			SingleUpdater::set(max, std::max(max, a));
		}

		inline void reset() {
			SingleUpdater::set(sum, 0UL);
			SingleUpdater::set(count, 0UL);
			SingleUpdater::set(min, ULONG_MAX);
			SingleUpdater::set(max, 0UL);
	  }
	};

  // StatAvgFloatItem: report float values, compute avg, min, max
	struct StatAvgFloatItem {
		double sum;
		unsigned long count;
		double min;
		double max;

    StatAvgFloatItem() : sum(0), count(0), min(INT_MAX), max(0) {}

		inline void record(double a) {
			SingleUpdater::inc(count);
			SingleUpdater::add(sum, a);
			SingleUpdater::set(min, std::min(min, a));
			SingleUpdater::set(max, std::max(max, a));
		}

		inline void reset() {
			SingleUpdater::set(sum, 0);
			SingleUpdater::set(count, 0);
			SingleUpdater::set(min, INT_MAX);
			SingleUpdater::set(max, 0);
	  }
	};

	struct StatHistItem {
  	std::map<unsigned long, unsigned long> histogram;

		inline void record(unsigned long pos) {
			HistUpdater::inc(histogram, pos);
		}

    inline void set(unsigned long pos, unsigned long a) {
    	HistUpdater::set(histogram, pos, a);
    }

		inline void reset() {
			HistUpdater::clear(histogram);
	  }
	};

	// step 1/2 ADD Items you want to count/report

	template <class T>
  static inline void printitem_abstract(
			std::ostream& os,
      const std::string& descr,
      const T& item,
			bool istrailing = false) {
		os << std::setw(25) << "\"" + descr + "sum\": \"" << item.sum
			<< "\", "
			<< std::setw(25) << "\"" + descr + "count\": \"" << item.count
			<< "\"," << std::setw(25) << "\"" + descr + "avg\": \""
      << (item.count > 0 ? (0.0 + item.sum) / item.count : item.count) << "\", "
			<< std::setw(25) << "\"" + descr + "min\": \"" << item.min
			<< "\", "
			<< std::setw(25) << "\"" + descr + "max\": \"" << item.max
			<< "\"";

		if (!istrailing) {
			os << ", ";
		}
	}

	static inline void printitem(
			std::ostream& os,
      const std::string& descr,
      const StatAvgItem& item,
			bool istrailing = false) {
		printitem_abstract(os, descr, item, istrailing);
	}

	static inline void printitem(
			std::ostream& os,
      const std::string& descr,
      const StatAvgFloatItem& item,
			bool istrailing = false) {
		printitem_abstract(os, descr, item, istrailing);
	}

	static inline void printitem(
			std::ostream& os,
      const std::string& descr,
      const StatItem& item,
			bool istrailing = false) {
		os << std::setw(25) << "\"" + descr + "\": \"" << item.value
			<< "\"";

		if (!istrailing) {
			os << ", ";
		}
	}

	static inline void printitem(
			std::ostream& os,
      const std::string& descr,
      const StatFloatItem& item,
			bool istrailing = false) {
		os << std::setw(25) << "\"" + descr + "\": \"" << item.value
			<< "\"";

		if (!istrailing) {
			os << ", ";
		}
	}

  static inline void printitem(
			std::ostream& os,
      const std::string& descr,
      const StatHistItem& item,
			bool istrailing = false) {
		unsigned long valsum{0};

    os << std::setw(25) << "\"" + descr + "hist\": [";

		if (!item.histogram.empty()) {
			auto it = item.histogram.cbegin();
			for (; it != std::prev(item.histogram.cend()); ++it) {
	      os << "[\"" << it->first << "\", \"" << it->second << "\"], ";
				valsum += it->second;
	    }
			os << "[\"" << it->first << "\", \"" << it->second << "\"] ";
			valsum += it->second;
		}
		os << "], ";

		os << std::setw(25) << "\"" << descr << "histsum\": \"" << valsum << "\"";

		if (!istrailing) {
			os << ", ";
		}
  }

	//step 2/2 Add items here such that it gets printed when the statistics object
	//is printed
};

template<
	typename SingleUpdater = RealUpdater,
	typename HistUpdater = MapUpdater>
class IndexStatistics : public Statistics<SingleUpdater, HistUpdater> {
public:
  // make nested classes visible
  using StatItem = typename Statistics<SingleUpdater, HistUpdater>::StatItem;
  using StatFloatItem =
		typename Statistics<SingleUpdater, HistUpdater>::StatFloatItem;
  using StatAvgItem =
		typename Statistics<SingleUpdater, HistUpdater>::StatAvgItem;
  using StatAvgFloatItem =
		typename Statistics<SingleUpdater, HistUpdater>::StatAvgFloatItem;
  // make inherited functions visible
  using Statistics<SingleUpdater, HistUpdater>::printitem;

	struct StatTimingItem {
		// common
		StatFloatItem nodeindex;
		StatFloatItem listdeletions;
		StatFloatItem listinsertions;
	};

	struct StatBuildTimingItem {
		StatFloatItem put;
		StatFloatItem prepareallocatelists;
		StatFloatItem preparecomputelistslengths;
		StatFloatItem preparelistappend;
		StatFloatItem preparenodeindexupdate;
		StatFloatItem preparesortalllists;
	};

  // index statistics
  StatFloatItem timing;
	StatFloatItem timingrename;
	StatFloatItem timingdelete;
	StatTimingItem timingdetails;
	StatBuildTimingItem timingbuilddetails;
  StatAvgItem invertedlists;
  StatAvgItem subtrees;

  friend std::ostream & operator<<(
      std::ostream& os,
      const IndexStatistics<SingleUpdater, HistUpdater>& statistics) {
    printitem(os, "timingindexing", statistics.timing);
		printitem(os, "timingrename", statistics.timingrename);
		printitem(os, "timingdelete", statistics.timingdelete);
		printitem(os, "timingdetails", statistics.timingdetails);
		printitem(os, "timingbuilddetails", statistics.timingbuilddetails);
    printitem(os, "invertedlists", statistics.invertedlists);
    printitem(os, "subtrees", statistics.subtrees, true);

		return os;
	}

	static inline void printitem(
			std::ostream& os,
			const std::string& descr,
			const StatTimingItem& item,
			bool istrailing = false) {
		printitem(os, descr + "nodeindex", item.nodeindex);
		printitem(os, descr + "listdeletions", item.listdeletions);
		printitem(os, descr + "listinsertions", item.listinsertions, istrailing);
	}

	static inline void printitem(
			std::ostream& os,
			const std::string& descr,
			const StatBuildTimingItem& item,
			bool istrailing = false) {
		printitem(os, descr + "put", item.put);
		printitem(os, descr + "prepareallocatelists", item.prepareallocatelists);
		printitem(
			os,
			descr + "preparecomputelistslengths",
			item.preparecomputelistslengths);
		printitem(os, descr + "preparelistappend", item.preparelistappend);
		printitem(os, descr + "preparenodeindexupdate", item.preparenodeindexupdate);
		printitem(
			os,
			descr + "preparesortalllists",
			item.preparesortalllists, istrailing);
	}
};

template<
	typename SingleUpdater = RealUpdater,
	typename HistUpdater = MapUpdater>
class FilterStatistics : public Statistics<SingleUpdater, HistUpdater> {
public:
  // make nested classes visible
  using StatItem = typename Statistics<SingleUpdater, HistUpdater>::StatItem;
  using StatFloatItem =
		typename Statistics<SingleUpdater, HistUpdater>::StatFloatItem;
  using StatAvgItem =
		typename Statistics<SingleUpdater, HistUpdater>::StatAvgItem;
  using StatHistItem =
		typename Statistics<SingleUpdater, HistUpdater>::StatHistItem;

  using Statistics<SingleUpdater, HistUpdater>::printitem;

  struct StatVerificationItem {
    StatItem total;
    StatItem duplicates;
    StatItem fresh;
  };

  struct StatLowerBoundItem {
    StatHistItem potential;
    StatHistItem real;
  };

	struct StatTimingItem {
		// common
		StatFloatItem initialization;
		StatFloatItem listretrieval;
		StatFloatItem rankingupdate;
		StatFloatItem nextlowerbound;
		StatFloatItem filtering;
		StatFloatItem total;

		// cone/shincone filter only
		StatFloatItem listsorting;
		StatFloatItem roundprologue;
		StatFloatItem roundepilogue;

		// lower bound merge filter only
		StatFloatItem sizesretrieval;
		StatFloatItem sizessorting;
		StatFloatItem inititalizationmergepositions;
		StatFloatItem overlapcomputation;
		StatFloatItem precandidateevaluation;

		// cone filter only
		StatFloatItem initializationpointers;

		// shincone filter only
		StatFloatItem upwardtraversal;
		StatFloatItem sizebucketevaluation;
		StatFloatItem parentsevaluation;
	};

  StatVerificationItem verifications;
  StatLowerBoundItem lower_bounds;
  StatTimingItem timings;
	StatHistItem subtreesizes;
	StatItem rounds;

  friend std::ostream & operator<<(
      std::ostream& os,
      const FilterStatistics<SingleUpdater, HistUpdater>& statistics) {
    printitem(os, "timing", statistics.timings);
    printitem(os, "verifications", statistics.verifications);
    printitem(os, "lowerbounds", statistics.lower_bounds);
		printitem(os, "subtreesizes", statistics.subtreesizes);
		printitem(os, "rounds", statistics.rounds, true);

		return os;
	}

  static inline void printitem(
			std::ostream& os,
      const std::string& descr,
      const StatVerificationItem& item,
			bool istrailing = false) {
    printitem(os, descr + "total", item.total);
    printitem(os, descr + "duplicates", item.duplicates);
    printitem(os, descr + "fresh", item.fresh, istrailing);
	}

  static inline void printitem(
			std::ostream& os,
      const std::string& descr,
      const StatLowerBoundItem& item,
			bool istrailing = false) {
    printitem(os, descr + "potential", item.potential);
    printitem(os, descr + "real", item.real, istrailing);
	}

	static inline void printitem(
			std::ostream& os,
			const std::string& descr,
			const StatTimingItem& item,
			bool istrailing = false) {
		printitem(os, descr + "initialization", item.initialization);
		printitem(os, descr + "listretrieval", item.listretrieval);
		printitem(os, descr + "rankingupdate", item.rankingupdate);
		printitem(os, descr + "nextlowerbound", item.nextlowerbound);
		printitem(os, descr + "filtering", item.filtering);
		printitem(os, descr + "total", item.total);

		printitem(os, descr + "listsorting", item.listsorting);
		printitem(os, descr + "roundprologue", item.roundprologue);
		printitem(os, descr + "roundepilogue", item.roundepilogue);

		printitem(os, descr + "sizesretrieval", item.sizesretrieval);
		printitem(os, descr + "sizessorting", item.sizessorting);
		printitem(
			os,
			descr + "inititalizationmergepositions",
			item.inititalizationmergepositions);
		printitem(os, descr + "overlapcomputation", item.overlapcomputation);
		printitem(os, descr + "precandidateevaluation", item.precandidateevaluation);
		printitem(os, descr + "initializationpointers", item.initializationpointers);

		printitem(os, descr + "upwardtraversal", item.upwardtraversal);
		printitem(os, descr + "sizebucketevaluation", item.sizebucketevaluation);
		printitem(
			os,
			descr + "parentsevaluation",
			item.parentsevaluation, istrailing);
	}
};

template<
	typename SingleUpdater = RealUpdater,
	typename HistUpdater = MapUpdater>
class GlobalFilterStatistics : public FilterStatistics<SingleUpdater, HistUpdater> {
public:
  // make nested classes visible
  using StatVerificationItem =
		typename FilterStatistics<SingleUpdater, HistUpdater>::StatVerificationItem;
  using StatLowerBoundItem =
		typename FilterStatistics<SingleUpdater, HistUpdater>::StatLowerBoundItem;
  // make inherited member functions visible
  using FilterStatistics<SingleUpdater, HistUpdater>::printitem;
  // make inherited member variables visible
  using FilterStatistics<SingleUpdater, HistUpdater>::verifications;
  using FilterStatistics<SingleUpdater, HistUpdater>::lower_bounds;
};

template<
	typename SingleUpdater = RealUpdater,
	typename HistUpdater = MapUpdater>
class RoundFilterStatistics : public FilterStatistics<SingleUpdater, HistUpdater> {
public:
  // make nested classes visible
  using StatItem =
		typename FilterStatistics<SingleUpdater, HistUpdater>::StatItem;
  using StatAvgItem =
		typename FilterStatistics<SingleUpdater, HistUpdater>::StatAvgItem;
  using StatAvgFloatItem =
		typename FilterStatistics<SingleUpdater, HistUpdater>::StatAvgFloatItem;
  using StatHistItem =
		typename FilterStatistics<SingleUpdater, HistUpdater>::StatHistItem;
  // make inherited functions visible
  using FilterStatistics<SingleUpdater, HistUpdater>::printitem;

  StatItem precandidates;
  StatItem rankingsize;
  StatItem rankingextended;
  StatItem rankingupdated;
  StatHistItem sizecache;
  StatItem candidates;
  StatItem lowerboundcacheverifications;
  StatHistItem lowerboundcache;
	StatAvgItem paths;

  inline void reset() {
    precandidates.reset();
    rankingsize.reset();
    rankingextended.reset();
    rankingupdated.reset();
    sizecache.reset();
    candidates.reset();
    lowerboundcacheverifications.reset();
    lowerboundcache.reset();
		paths.reset();
  }

  friend std::ostream & operator<<(
      std::ostream& os,
      const RoundFilterStatistics<SingleUpdater, HistUpdater>& statistics) {
    printitem(os, "precandidates", statistics.precandidates);
    printitem(os, "rankingsize", statistics.rankingsize);
    printitem(os, "rankingextended", statistics.rankingextended);
    printitem(os, "rankingupdated", statistics.rankingupdated);
    printitem(os, "candidates", statistics.candidates);
    printitem(
			os,
			"lowerboundcacheverifications",
			statistics.lowerboundcacheverifications);
    printitem(os, "sizecache", statistics.sizecache);
    printitem(os, "lowerboundcache", statistics.lowerboundcache);
		printitem(os, "paths", statistics.paths, true);

		return os;
	}
};

template<
	typename SingleUpdater = RealUpdater,
	typename HistUpdater = MapUpdater>
class MultiRoundFilterStatistics {
public:
	using RoundFilterStats = RoundFilterStatistics<SingleUpdater, HistUpdater>;
	std::map<unsigned long, RoundFilterStats> rounds;

	inline void set(unsigned long round, RoundFilterStats& statistics) {
		HistUpdater::set(rounds, round, statistics);
	}

	friend std::ostream & operator<<(
	    std::ostream& os,
	    const MultiRoundFilterStatistics<SingleUpdater, HistUpdater>& statistics)
	{
		printitem(os, "rounds", statistics.rounds, true);

    return os;
	}

	static inline void printitem(
			std::ostream& os,
      const std::string& descr,
      const std::map<unsigned long, RoundFilterStats>& item,
			bool istrailing = false) {
		if (!item.empty()) {
			auto it = item.cbegin();
			for (; it != std::prev(item.cend()); ++it) {
	      os << "\"" << it->first << "\": {";
	      os << it->second << "},";
      }
			os << "\"" << it->first << "\": {" << it->second << "}";

			if (!istrailing) {
				os << ", ";
			}
    }
	}
};

} // namespace common

#endif // COMMON_STATISTICS_H
