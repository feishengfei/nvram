#ifndef __NVRAM_RULE_H_
#define __NVRAM_RULE_H_

#define NVRAM_BUF_LEN          8192

#define NVRAM_USE_CLI          0
#define NVRAM_USE_WEB          1

#define NVRAM_INVALID          -1
#define NVRAM_VAL_TRUNC        -2
#define NVRAM_NO_RULE_SET      -3
#define NVRAM_NO_RULE          -4
#define NVRAM_NO_ATTRIBUTE     -5
#define NVRAM_IDX_OUT_RANGE    -6

#define RULE_SEP "|"
#define ATTR_SEP "^"

struct attr {
    char *name;
    int (*func_orig)(char *, char *, int);
    int (*func_wrap)(char *, char *, int);
};

enum opcode {
    ADD_RULE = 0,
    DELETE_RULE = 1,
    REPLACE_RULE = 2
};

/******************************************/
int show_enable_value(char *val, char *buf, int bsize);
int show_value(char *val, char *buf, int bsize);
int show_if_value(char *val, char *buf, int bsize);
int show_proto_value(char *val, char *buf, int bsize);
int show_mask_value(char *val, char *buf, int bsize);

int sep_string(char *word, const char *delim, char **idx_arr, int max_tok);

/**
 * \brief Get a specified rule set from nvram, parse it by the RULE_SEP 
 * character, and return the nth rule.
 */
int nvram_get_rule(const char *rule_set, int nth, 
	char *buf, int bsize);

/**
 * \brief Get a specified specified rule set from nvram, parse it by 
 * RULE_SEP character, and return the subrule with the attribute position in
 * between start and end.
 */
int nvram_get_subrule(const char *rule_set, int nth, 
	int start, int end, char *buf, int bsize);

/**
 * \brief Get a specified attribute of the given rule from the rule set queried 
 * from nvram. The rules are separated by blank character and the attributes
 * in a rule are separated by '-' character.
 */
int nvram_get_attr_val(const char *rule_set, int nth, 
	const char *type, char *buf, int bsize, int use);

int nvram_op_rule(const char *rule_set, enum opcode op, 
	int nth, char *new_rule);

/**
 * \brief Replace the nth rule in the given rule-name as the specific rule.
 */
int nvram_replace_rule(const char *rule_set, int nth, char *new_rule);

/**
 * \brief Replace the mth attribute of the nth rule in the given rule-name as 
 * the specific rule.
 */
int nvram_replace_attr(const char *rule_set, int nth, 
	char *attr, char *new_attr);

/**
 * \brief Append a new rule into the given rule-name.
 */
int nvram_append_rule(const char *rule_set, char *new_rule);

/**
 * \brief Prepend a new rule into the given rule-name.
 */
int nvram_prepend_rule(const char *rule_set, char *new_rule);

/**
 * \brief Add a new rule in the given rule-name as the specific rule.
 */
int nvram_add_rule(const char *rule_set, int nth, char *new_rule);

/**
 * \brief Delete nth rule in the given rule-name.
 */
int nvram_delete_rule(const char *rule_set, int nth);

/**
 * \brief Get the number of rules in the given rule-name.
 */
int nvram_get_rule_num(const char *rule_set);
#endif
