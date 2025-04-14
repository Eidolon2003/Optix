#include "ini.h"
#include <fstream>

int read_ini(Options* config, std::string filename) {
	std::ifstream file(filename);
	if (!file.is_open()) return 1;

	while (!file.eof()) {
		std::string str;
		std::getline(file, str);
		size_t eq = str.find('=');
		if (eq == std::string::npos) continue;
		std::string key = str.substr(0, eq);
		int value = std::stoi(str.substr(eq + 1));
		config->emplace(key, value);
	}

	return 0;
}

void write_ini(Options* config, std::string filename) {
	std::ofstream file(filename, std::ios::trunc);

	for (auto x : OPTS) {
		file << x.name << '=' << config->at(x.name) << '\n';
	}
}

void load_defaults(Options* config) {
	for (auto x : OPTS) {
		config->emplace(x.name, x.def);
	}
}

void check_bounds(Options* config) {
	for (auto x : OPTS) {
		config->at(x.name) = std::max(config->at(x.name), x.min);
	}
}