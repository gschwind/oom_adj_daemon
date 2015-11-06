
#ifndef _CONFIG_PARSER_HXX_
#define _CONFIG_PARSER_HXX_

#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <regex>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <boost/regex.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>

#include "rule.hxx"
#include "rule_user.hxx"
#include "rule_group.hxx"

using namespace std;

vector<string> split(string const & s, boost::regex const & r) {
	vector<string> ret;
	boost::sregex_token_iterator i{s.begin(), s.end(), r, -1};
	for (; i != boost::sregex_token_iterator{}; ++i) {
		ret.push_back(i->str());
	}
	return ret;
}

class config_parser_t {
	using func_t = function<shared_ptr<rule_t>(vector<string> const &)>;
	map<string, func_t> create_rule_map;
public:

	config_parser_t() {
		create_rule_map["user"] = &rule_user_t::create_rule;
		create_rule_map["group"] = &rule_group_t::create_rule;
		//create_rule_map["skip_user"] = &rule_skip_user_t::create_rule;
	}

	vector<shared_ptr<rule_t>> parse(char const * filename) {
		vector<shared_ptr<rule_t>> ret;
		boost::regex space{"\\s+"};
		ifstream in(filename, ios::in);
		string line;
		int line_no = 0;

		for(auto & i: create_rule_map) {
			printf("cmd[0] = '%s'\n", i.first.c_str());
		}

		while(!in.eof()) {
			getline(in, line);
			line_no += 1;
			if(line.size() == 0)
				continue;
			if(line[0] == '#')
				continue;

			auto cmd = split(line, space);

			if(cmd.size() == 0)
				continue;

			printf("cmd[0] = '%s'\n", cmd[0].c_str());
			auto handler = create_rule_map.find(cmd[0]);
			if(handler != create_rule_map.end()) {
				auto rule = handler->second(cmd);
				if(rule != nullptr) {
					ret.push_back(rule);
				} else {
					printf("Invalid rule line %d: %s\n", line_no, line.c_str());
				}
			} else {
				printf("Invalid rule line %d: %s\n", line_no, line.c_str());
				continue;
			}

		}

		return ret;
	}

};


#endif
