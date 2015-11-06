/*
 * rule.hxx
 *
 * copyright (2015) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#ifndef RULE_USER_HXX_
#define RULE_USER_HXX_

#include <vector>
#include "rule.hxx"

using namespace std;

class rule_user_t : public rule_t {
	uid_t uid;
	int adj;

public:

	static shared_ptr<rule_t> create_rule(vector<string> const & args) {
		boost::regex const user_name{"[_a-zA-Z]\\w*"};
		boost::regex const user_id{"\\d+"};
		boost::regex const number{"[+-]?\\d+"};

		if(args.size() != 3)
			return nullptr;

		if(not boost::regex_match(args[2], number))
			return nullptr;

		int adj = stoi(args[2]);

		if (boost::regex_match(args[1], user_name)) {
			struct passwd * pwd;
			pwd = getpwnam(args[1].c_str());
			if(pwd != 0) {
				return make_shared<rule_user_t>(pwd->pw_uid, adj);
			} else {
				std::cout << "not found user id: " << args[1] << std::endl;
				return nullptr;
			}
		} else if (boost::regex_match(args[1], user_id)) {
			uid_t uid = stoi(args[1]);
			return make_shared<rule_user_t>(uid, adj);
		}

		return nullptr;
	}

	rule_user_t(int uid, int adj) : uid(uid), adj(adj) { }
	virtual ~rule_user_t() { }

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


#endif /* RULE_USER_HXX_ */
