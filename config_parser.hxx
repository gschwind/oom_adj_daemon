
#ifndef _CONFIG_PARSER_HXX_
#define _CONFIG_PARSER_HXX_

#include <vector>
#include <memory>
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


using namespace std;

class rule_t {
public:
	virtual ~rule_t() { }
	virtual bool check_rule(int pid) = 0;
};

class group_rule_t : public rule_t {
	gid_t gid;
	int adj;

public:

	group_rule_t(int gid, int adj) : gid(gid), adj(adj) { }
	virtual ~group_rule_t() { }

	virtual bool check_rule(int pid) {
		char buf0[1024];
		char buf1[1024];
		struct stat st;
		int ret;
		gid_t groups[1024];
		int ngroups[1];
		ngroups[0] = 1024;
		struct passwd * pwd;

		snprintf(buf0, 1024, "/proc/%d", pid);

		ret = stat(buf0, &st);
		if(ret != 0) {
			return false;
		}

		pwd = getpwuid(st.st_uid);
		if(pwd == 0) {
			return false;
		}

		getgrouplist(pwd->pw_name, pwd->pw_gid, groups, ngroups);

		for(int i = 0; i < *ngroups; ++i) {
			if(groups[i] == gid) {
				snprintf(buf0, 1024, "%d", adj);
				snprintf(buf1, 1024, "/proc/%d/oom_score_adj", pid);
				int fd = open(buf1, O_WRONLY);
				write(fd, buf0, strlen(buf0));
				close(fd);
				return true;
			}
		}

		return false;

	}

};

class user_rule_t : public rule_t {
	uid_t uid;
	int adj;

public:
	user_rule_t(int uid, int adj) : uid(uid), adj(adj) { }
	virtual ~user_rule_t() { }

	virtual bool check_rule(int pid) {
		char buf0[1024];
		char buf1[1024];
		struct stat st;
		int ret;

		/* get user id of current process */
		snprintf(buf0, 1024, "/proc/%d", pid);

		ret = stat(buf0, &st);
		if(ret != 0) {
			return false;
		}

		/* if the user id match the rule, apply the rule */
		if(st.st_uid == uid) {
			snprintf(buf0, 1024, "%d", adj);
			snprintf(buf1, 1024, "/proc/%d/oom_score_adj", pid);
			int fd = open(buf1, O_WRONLY);
			write(fd, buf0, strlen(buf0));
			close(fd);
			return true;
		}

		return false;
	}


};

enum token_type_e {
	NUMBER,
	USER_NAME,
	GROUP_NAME,
	GROUP_ID
};

struct token_handler_t {
	token_type_e type;
	int value;
	token_handler_t(token_type_e type, int value) : type(type), value(value) { }
};

vector<string> split(string const & s, boost::regex const & r) {
	vector<string> ret;
	vector<int> x;
//	sregex_token_iterator i{s.begin(), s.end(), r, -1};
//	for (; i != sregex_token_iterator{}; ++i) {
//		ret.push_back(i->str());
//	}

	boost::smatch m;
	boost::regex_search(s, m, r);

	x.push_back(0);
	for(unsigned i = 0; i < m.size(); ++i) {
		int pos = m.position(i);
		int len = pos + m.length(i);
		if (x[x.size() - 1] < pos) {
			x.push_back(pos);
			x.push_back(len);
		}
	}
	x.push_back(s.size());

	for(unsigned i = 0; i < x.size(); i += 2) {
		//cout << string({&(s[x[i]]), &(s[x[i+1]])}) << endl;
		ret.push_back(string({&(s[x[i]]), &(s[x[i+1]])}));
	}

	return ret;
}

class config_parser_t {
	vector<token_handler_t> stack;
	vector<shared_ptr<rule_t>> rules;
public:
	vector<shared_ptr<rule_t>> get_rules() { return rules; }


	config_parser_t(char const * filename) {
		boost::regex space{"\\s+"};
		boost::regex group_name{"@[_a-zA-Z]\\w*"};
		boost::regex group_id{"@\\d+"};
		boost::regex user_name{"[_a-zA-Z]\\w*"};
		boost::regex user_id{"\\d+"};
		boost::regex number{"[+-]?\\d+"};

		ifstream in(filename, ios::in);
		string line;

		while(!in.eof()) {
			getline(in, line);
			if(line.size() == 0)
				continue;
			if(line[0] == '#')
				continue;

			auto x = split(line, space);

			if(x.size() != 2) {
				cout << "ignore line: " << line << endl;
				continue;
			}

			if(not regex_match(x[1], number)) {
				cout << "ignore line: " << line << endl;
				continue;
			}

			int adj = stoi(x[1]);

			if(boost::regex_match(x[0], group_name)) {
				struct group * grp;
				grp = getgrnam(&(x[0].c_str())[1]);
				if(grp != 0) {
					rules.push_back(make_shared<group_rule_t>(grp->gr_gid, adj));
				} else {
					std::cout << "not found group id: " << x[0] << std::endl;
				}
			} else if (boost::regex_match(x[0], group_id)) {
				gid_t gid = atoi(&(x[0].c_str())[1]);
				rules.push_back(make_shared<group_rule_t>(gid, adj));
			} else if (boost::regex_match(x[0], user_name)) {
				struct passwd * pwd;
				pwd = getpwnam(x[0].c_str());
				if(pwd != 0) {
					rules.push_back(make_shared<user_rule_t>(pwd->pw_uid, adj));
				} else {
					std::cout << "not found user id: " << x[0] << std::endl;
				}
			} else if (boost::regex_match(x[0], user_id)) {
				uid_t uid = stoi(x[0]);
				rules.push_back(make_shared<user_rule_t>(uid, adj));
			}
		}
	}

};


#endif
