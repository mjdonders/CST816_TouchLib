#include "Tools.h"

#include <Arduino.h>
#include <limits.h>

namespace MDO {

/**
 * Determines the difference in time between two millis calls, assuming to be 'short'.
 * (where short < half the overflow time, so roughly 25 days)
 */
/*static*/ unsigned long Tools::millisDiff(const unsigned long& ulStart, const unsigned long& ulEnd) {
	unsigned long ulDiff = 0;
	if (ulStart <= ulEnd) {
		//normal easy scenario
		ulDiff = ulEnd - ulStart;
	} else {
		//an overflow has occured, so ulStart > ulEnd
		ulDiff = ulEnd + (ULONG_MAX - ulStart);
	}

	return ulDiff;
}

/**
 * Determines the difference in time between 'now' and the provided start time (which should also be a result from millis), assuming to be 'short'.
 * (where short < half the overflow time, so roughly 25 days)
 */
/*static*/ unsigned long Tools::millisDiff(const unsigned long& ulStart) {
	return millisDiff(ulStart, millis());
}

Tools::Tools() {
}

Tools::~Tools() {
}

}	//namespace end