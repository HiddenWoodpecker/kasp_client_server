#pragma once

#include <stdint.h>
#include <string>
#include <vector>
namespace server {

class Config {
public:
	Config();

	bool load(const std::string &path);

	const std::string &getConfigPath() const { return _configPath; }
	void setConfigPath(const std::string &path) { _configPath = path; }

	uint16_t getPort() const { return _port; }
	void setPort(uint16_t port) { _port = port; }

	const std::vector<std::string> &getPatterns() const { return _patterns; }
	void setPatterns(const std::vector<std::string> &patterns) {_patterns = patterns;}

private:
	std::string _configPath;
	uint16_t _port;
	std::vector<std::string> _patterns;
};

} // namespace server
