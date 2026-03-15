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

enum class MessageType : uint8_t {
	fileData = 1,
	scanResult = 2,
	statsRequest = 3,
	statsResponse = 4
};

struct ScanResult {
	bool isInfected;
	uint32_t patternsFound;
	std::vector<std::string> foundPatterns;

	ScanResult() : isInfected(false), patternsFound(0) {}
};

} // namespace protocol
