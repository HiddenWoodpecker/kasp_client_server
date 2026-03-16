#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace protocol {

struct MessageHeader {
	uint32_t size;

	MessageHeader() : size(0) {}
	explicit MessageHeader(uint32_t s) : size(s) {}
};


struct ScanResult {
	bool isInfected;
	uint32_t patternsFound;
	std::vector<std::string> foundPatterns;

	ScanResult() : isInfected(false), patternsFound(0) {}
};

} // namespace protocol
