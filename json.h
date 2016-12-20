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

lexem_t get_next_lexem(FILE *stream);
extern double g_number;
extern char g_string[];

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

#endif /* JSON_H */
