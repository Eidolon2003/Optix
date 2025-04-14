#pragma once

#include <map>
#include <string>

static const struct {
	const char* name;
	int def;
	int min;
} OPTS[] = {
	{"width", 640, 320},
	{"height", 480, 240},
	{"framerate", 60, 0},
	{"fullscreen", 0, 0}
};

typedef std::map<std::string, int> Options;

int read_ini(Options*, std::string);	//returns 0 on success
void write_ini(Options*, std::string);
void load_defaults(Options*);
void check_bounds(Options*);