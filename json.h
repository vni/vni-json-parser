#ifndef JSON_H
#define JSON_H

/*** LEXER *******************************************************************************************************/

typedef enum lexem {
	L_END,
	L_INVALID,
	L_INVALID_STRING,
	L_STRING_TOO_BIG,
	L_COMMA,
	L_COLON,
	L_NULL,
	L_TRUE,
	L_FALSE,
	L_NUMBER,
	L_STRING,
	L_LBRACKET,
	L_RBRACKET,
	L_LBRACE,
	L_RBRACE
} lexem_t;

lexem_t get_next_lexem(const char **input);
extern double g_number;
extern char g_string[];
extern char *lexem_to_string[];

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

extern char *type_to_string[];

node_t *parse_stream(const char **input);

/*** AUXILIARY ***************************************************************************************************/
node_t *create_node(type_t t, ...);
void dump_tree(node_t *root, int indent);

#endif /* JSON_H */
