/*
 * rule_group.hxx
 *
 *  Created on: 6 nov. 2015
 *      Author: gschwind
 */

#ifndef RULE_GROUP_HXX_
#define RULE_GROUP_HXX_

#include <vector>
#include "rule.hxx"

using namespace std;

class rule_group_t : public rule_t {
	gid_t gid;
	int adj;

public:

	static shared_ptr<rule_t> create_rule(vector<string> const & args) {
		boost::regex const group_name{"[_a-zA-Z]\\w*"};
		boost::regex const group_id{"\\d+"};
		boost::regex const number{"[+-]?\\d+"};

		if(args.size() != 3)
			return nullptr;

		if(not boost::regex_match(args[2], number))
			return nullptr;

		int adj = stoi(args[2]);

		if(boost::regex_match(args[1], group_name)) {
			struct group * grp;
			grp = getgrnam(args[1].c_str());
			if(grp != 0) {
				return make_shared<rule_group_t>(grp->gr_gid, adj);
			} else {
				cout << "not found group id: " << args[1] << endl;
				return nullptr;
			}
		} else if (boost::regex_match(args[1], group_id)) {
			gid_t gid = stoi(args[1].c_str());
			return make_shared<rule_group_t>(gid, adj);
		}

		return nullptr;
	}


	rule_group_t(int gid, int adj) : gid(gid), adj(adj) { }
	virtual ~rule_group_t() { }

	virtual bool apply_rule(struct proc_event const * ev) {

		switch(ev->what){
		case proc_event::PROC_EVENT_FORK:
			return _apply_rule(ev->event_data.fork.child_pid);
			break;
		case proc_event::PROC_EVENT_EXEC:
			return _apply_rule(ev->event_data.exec.process_pid);
			break;
		case proc_event::PROC_EVENT_EXIT:
			return false;
			break;
		case proc_event::PROC_EVENT_UID:
			return _apply_rule(ev->event_data.exec.process_pid);
			break;
		default:
			return false;
			break;
		}

	}

	bool _apply_rule(int pid) {
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



#endif /* RULE_GROUP_HXX_ */
