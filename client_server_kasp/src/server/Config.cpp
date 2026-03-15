#include "../../include/server/Config.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace server {

Config::Config() : _port(0) {}

bool Config::load(const std::string& path) {
	_configPath = path;
	
	std::ifstream file(_configPath);
	if (!file.is_open()) {
		std::cerr << ": " << _configPath << std::endl;
		return false;
	}
	
	try {
		nlohmann::json json;
		file >> json;
		
		if (json.contains("patterns")) {
			_patterns = json["patterns"].get<std::vector<std::string>>();
		}
	} catch (const std::exception& e) {
		std::cerr << "config parse error: " << e.what() << std::endl;
		return false;
	}
	
	return true;
}

} // namespace server
