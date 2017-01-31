#ifndef JSON_H
#define JSON_H

/*** PARSER ******************************************************************************************************/

typedef enum type {
	T_NULL,
	T_FALSE,
	T_TRUE,
	T_STRING,
	T_NUMBER,
	T_ARRAY,
	T_OBJECT
} type_t;

typedef struct node {
	struct node *next;
	struct node *child;
	type_t type;
	union {
		char *string;
		double number;
	} u;
} node_t;

extern const char *type_to_string[];

node_t *parse_stream(const char **input);

/*** auxiliary functions *****************************************************************************************/
node_t *create_node(type_t t, ...);
void dump_tree(node_t *root, int indent);
void free_tree(node_t *root);
int compare_trees(node_t *t1, node_t *t2);

void dump_out_stream(node_t *root);

#endif /* JSON_H */
