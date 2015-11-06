/*
 * rule.hxx
 *
 * copyright (2015) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#ifndef RULE_HXX_
#define RULE_HXX_

/* rules handler API */
class rule_t {
public:
	virtual ~rule_t() { }

	/**
	 * apply the rule for this event, return true if
	 * the following rules must be applyed.
	 **/
	virtual bool apply_rule(struct proc_event const * ev) = 0;

};



#endif /* RULE_HXX_ */
