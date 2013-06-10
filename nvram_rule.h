#ifndef __NVRAM_RULE_H_
#define __NVRAM_RULE_H_

#define EZPLIB_BUF_LEN          8192

#define EZPLIB_USE_CLI          0
#define EZPLIB_USE_WEB          1

#define EZPLIB_INVALID          -1
#define EZPLIB_VAL_TRUNC        -2
#define EZPLIB_NO_RULE_SET      -3
#define EZPLIB_NO_RULE          -4
#define EZPLIB_NO_ATTRIBUTE     -5
#define EZPLIB_IDX_OUT_RANGE    -6

#define RULE_SEP "|"
#define ATTR_SEP "^"

struct attr {
    char *name;
    int (*func_orig)(char *, char *, char *, int);
    int (*func_wrap)(char *, char *, char *, int);
};

/******************************************/
int show_enable_value(char *type, char *val, char *buf, int bsize);
int show_value(char *type, char *val, char *buf, int bsize);
int show_if_value(char *type, char *val, char *buf, int bsize);
int show_proto_value(char *type, char *val, char *buf, int bsize);
int show_mask_value(char *type, char *val, char *buf, int bsize);

int sep_string(char *word, const char *delim, char **idx_arr, int max_tok);

/**
 * \brief Get a specified rule set from nvram, parse it by the RULE_SEP 
 * character, and return the nth rule.
 */
int ezplib_get_rule(char *rule_set, int nth, 
	char *buf, int bsize);

/**
 * \brief Get a specified specified rule set from nvram, parse it by 
 * RULE_SEP character, and return the subrule with the attribute position in
 * between start and end.
 */
int ezplib_get_subrule(char *rule_set, int nth, 
	int start, int end, char *buf, int bsize);

/**
 * \brief Get a specified attribute of the given rule from the rule set queried 
 * from nvram. The rules are separated by blank character and the attributes
 * in a rule are separated by '-' character.
 */
int ezplib_get_attr_val(char *rule_set, int nth, 
	char *type, char *buf, int bsize, int use);

//int ezplib_op_rule(char *rule_set, enum opcode op, int nth, char *new_rule);

/**
 * \brief Replace the nth rule in the given rule-name as the specific rule.
 */
int ezplib_replace_rule(char *rule_set, int nth, char *new_rule);

/**
 * \brief Replace the mth attribute of the nth rule in the given rule-name as 
 * the specific rule.
 */
int ezplib_replace_attr(char *rule_set, int nth, 
	char *attr, char *new_attr);

/**
 * \brief Append a new rule into the given rule-name.
 */
int ezplib_append_rule(char *rule_set, char *new_rule);

/**
 * \brief Prepend a new rule into the given rule-name.
 */
int ezplib_prepend_rule(char *rule_set, char *new_rule);

/**
 * \brief Add a new rule in the given rule-name as the specific rule.
 */
int ezplib_add_rule(char *rule_set, int nth, char *new_rule);

/**
 * \brief Delete nth rule in the given rule-name.
 */
int ezplib_delete_rule(char *rule_set, int nth);

/**
 * \brief Get the number of rules in the given rule-name.
 */
int ezplib_get_rule_num(char *rule_set);
#endif
