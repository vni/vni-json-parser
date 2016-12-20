/* FIXME: Added doxygen documentation to all the functions. */
/* FIXME: Add tests. */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "json.h"

/* FIXME: Design it as a library:
 * no program exiting. Just report errors.
 *
 * From lexer return lexems AND errors in the input.
 * No printing to the stdout or stderr:
 * IT IS A LIBRARY.
 */

void die(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

void *xmalloc(unsigned sz) {
	void *p = malloc(sz);
	if (p == NULL) {
		fprintf(stderr, "Failed to allocate memory. Exiting...\n");
		exit(EXIT_FAILURE);
	}
	return p;
}

void *xcalloc(unsigned sz, unsigned n) {
	void *p = calloc(sz, n);
	if (p == NULL) {
		fprintf(stderr, "Failed to allocate memory. Exiting...\n");
		exit(EXIT_FAILURE);
	}
	return p;
}

char *xstrdup(const char *s) {
	unsigned n = strlen(s) + 1;
	char *newstr = xmalloc(n);
	return memcpy(newstr, s, n);
}

/******************************************************************************************/

#define MAX_STRING_LEXEM_LEN 2048

double g_number;
char g_string[MAX_STRING_LEXEM_LEN];

int char_to_hex(int c) {
	if (isdigit(c))
		return c - '0';
	else
		return toupper(c) - 'A' + 10;
}

int get_4digit_hex(char *g_string, FILE *stream) {
	/* Assume the g_string points to the buffer that has
	 * at least 2 bytes */

	unsigned short v = 0;

	int i;
	for (i = 0; i < 4; i++) {
		int c = fgetc(stream);
		if (!isxdigit(c)) {
			fprintf(stderr, "expected hex digit after \\u\n");
			return -1;
		}

		v = (v << 4) + char_to_hex(c);
	}

	/*fprintf(stderr, "%s: v: 0x%04X\n", __FUNCTION__, v);*/

	if (v < 0x7F) {
		// 0000'0000 0111'1111
		*g_string++ = v & 0x7F;
		/*fprintf(stderr, "(v < 0x7F) 0x%02X\n", (unsigned char)g_string[-1]);*/
		return 1;
	} else if (v < 0x07FF) {
		// 0000'0111 1111'1111
		*g_string++ = 0xC0 | ((v >> 6) & 0x1F); // take 5 bits
		*g_string++ = 0x80 | (v & 0x3F); // take 6 bits
		/*fprintf(stderr, "(v < 0x07FF) 0x%02X 0x%02X\n", (unsigned char)g_string[-2], (unsigned char)g_string[-1]);*/
		return 2;
	} else {
		// 1111'1111 1111'1111
		*g_string++ = 0xE0 | ((v >> 12) & 0x0F);
		*g_string++ = 0x80 | ((v >> 6) & 0x3F);
		*g_string++ = 0x80 | (v & 0x03F);
		/*fprintf(stderr, "(v < 0xFFFF) 0x%02X 0x%02X 0x%02X\n", (unsigned char)g_string[-3], (unsigned char)g_string[-2], (unsigned char)g_string[-1]);*/
		return 2;
	}
}

lexem_t get_next_lexem(FILE *stream) {
	int c;

	while (isspace(c = fgetc(stream)))
		;

	switch (c) {
		case EOF: return L_END;
		case ',': return L_COMMA;
		case ':': return L_COLON;
		case '[': return L_LBRACKET;
		case ']': return L_RBRACKET;
		case '{': return L_LBRACE;
		case '}': return L_RBRACE;
		/* no default: */
	}

	// FIXME: '.' started float numbers are not conformant to JSON spec
	if (isdigit(c) || c == '-' || c == '.') {
		int r;
		ungetc(c, stream);
		r = fscanf(stream, "%lf", &g_number);
		if (r != 1) {
			fprintf(stderr, "invalid input...");
			return L_INVALID;
		}

		return L_NUMBER;
	}

	if (c == '"') {
		/* FIXME: add support for \ sequence */
		int i = 0;
		while ((c = fgetc(stream)) != '"' && c != EOF) {
			if (i >= MAX_STRING_LEXEM_LEN) {
				fprintf(stderr, "Input error: too big string lexem on input.\n");
				return L_STRING_TOO_BIG;
			}
			g_string[i] = c;

			if (c == '\\') {
				c = fgetc(stream);
				if (c == EOF) {
					fprintf(stderr, "Input error: EOF right after '\\'.\n");
					return L_INVALID_STRING;
				}

				switch (c) {
					case '\"': g_string[i] = '\"'; break;
					case '\\': g_string[i] = '\\'; break;
					case '/': g_string[i] = '/'; break;
					case 'b': g_string[i] = '\b'; break;
					case 'f': g_string[i] = '\f'; break;
					case 'n': g_string[i] = '\n'; break;
					case 'r': g_string[i] = '\r'; break;
					case 't': g_string[i] = '\t'; break;
					case 'u': {
								  if (i >= MAX_STRING_LEXEM_LEN-3) {
									  /* \u can be converted into 3 bytes,
									   * check available string size */
									  return L_STRING_TOO_BIG;
								  }

								  int ret = get_4digit_hex(&g_string[i], stream);
								  if (ret == -1)
									  return L_INVALID_STRING;
							  }
							  break;
					default: fprintf(stderr, "Bad backslash character: '\\%c'\n", c); return L_INVALID_STRING;
				}
			}

			i++;
		}
		g_string[i] = '\0';

		if (c == EOF) {
			/* EOF in the middle of the string */
			return L_INVALID;
		}

		return L_STRING;
	}

	if (c == 'n' || c == 't' || c == 'f') {
		ungetc(c, stream);

		char lexem[16] = {0};
		fscanf(stream, "%15s", lexem);

		/*fprintf(stderr, "lexem: '%s'\n", lexem);*/

		if (strcmp(lexem, "null") == 0) {
			return L_NULL;
		} else if (strcmp(lexem, "false") == 0) {
			return L_FALSE;
		} else if (strcmp(lexem, "true") == 0) {
			return L_TRUE;
		} else {
			printf("L_INVALID: lexem: '%s'\n", lexem);
			/* FIXME: CONTINUE HERE FROM HERE. */
			return L_INVALID;
			/*die("invalid lexem in input stream: '%s'\n", lexem);*/
		}
	}

	/* invalid input, can't handle it */
	return L_INVALID;
}

char *lexem_to_string[] = {
	"L_END",
	"L_INVALID",
	"L_INVALID_STRING",
	"L_STRING_TOO_BIG",
	"L_COMMA",
	"L_COLON",
	"L_NULL",
	"L_TRUE",
	"L_FALSE",
	"L_NUMBER",
	"L_STRING",
	"L_LBRACKET",
	"L_RBRACKET",
	"L_LBRACE",
	"L_RBRACE"
};

/************************************************************************************************/

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

void expect(lexem_t l, lexem_t expected) {
	if (l != expected) {
		fprintf(stderr, "Expected %s but got: %s\n", lexem_to_string[expected], lexem_to_string[l]);
		exit(EXIT_FAILURE);
	}
}

/*
void expect(FILE *input, lexem_t expected) {
	lexem_t l = get_next_lexem(input);
	if (l != expected) {
		fprintf(stderr, "Expected %s but got: %s\n", lexem_to_string[expected], lexem_to_string[l]);
		exit(EXIT_FAILURE);
	}
}
*/

node_t *create_node(FILE *input_stream, type_t t, ...) {
	node_t *n = xcalloc(sizeof(node_t), 1);

	n->type = t;

	switch (t) {
		case T_NUMBER: {
			va_list args;
			va_start(args, t);
			n->u.number = va_arg(args, double);
			va_end(args);
			break;
		}
		case T_STRING: {
			va_list args;
			va_start(args, t);
			n->u.string = xstrdup(va_arg(args, char *));
			va_end(args);
			break;
		}
	}

	return n;
}

node_t *parse_object(FILE *input);

node_t *parse_array(FILE *input) {
	lexem_t l;

	node_t *node = NULL;

	l = get_next_lexem(input);
	switch (l) {
		case L_NULL: node = create_node(input, T_NULL); break;
		case L_FALSE: node = create_node(input, T_FALSE); break;
		case L_TRUE: node = create_node(input, T_TRUE); break;
		case L_NUMBER: node = create_node(input, T_NUMBER, g_number); break;
		case L_STRING: node = create_node(input, T_STRING, g_string); break;
		case L_LBRACKET: node = create_node(input, T_ARRAY);
						 node->child = parse_array(input);
						 break;
		case L_LBRACE: node = create_node(input, T_OBJECT);
					   node->child = parse_object(input);
					   break;
		/* no default: */
	}

	l = get_next_lexem(input);
	if (l == L_COMMA) {
		node->next = parse_array(input);
		return node;
	}

	expect(l, L_RBRACKET);
	return node;
}

node_t *parse_object(FILE *input) {
	return NULL;
}

node_t *parse_stream(FILE *input) {
	lexem_t l;
	node_t *root;

	l = get_next_lexem(input);
	switch (l) {
		case L_NULL: root = create_node(input, T_NULL); break;
		case L_FALSE: root = create_node(input, T_FALSE); break;
		case L_TRUE: root = create_node(input, T_TRUE); break;
		case L_NUMBER: root = create_node(input, T_NUMBER, g_number); break;
		case L_STRING: root = create_node(input, T_STRING, g_string); break;
		case L_LBRACKET: root = create_node(input, T_ARRAY);
						 root->child = parse_array(input);
						 break;
		case L_LBRACE: root = create_node(input, T_OBJECT);
					   root->child = parse_object(input);
					   break;
		default: fprintf(stderr, "Unexpected lexem %s in input stream.\n", lexem_to_string[l]); exit(EXIT_FAILURE); break;
	}

 
	l = get_next_lexem(input);
	expect(l, L_END);

	return root;
}

/************************************************************************************************/

void dump_tree(node_t *root, int indent) {
	node_t *node;
	int i;

	printf("\ndump_tree(root = %p)\n\n", root);

	for (i = 0; i < indent; i++)
		printf("    ");

	for (node = root; node; node = node->next) {
		switch (node->type) {
			case T_NULL: printf("null"); break;
			case T_FALSE: printf("false"); break;
			case T_TRUE: printf("true"); break;
			case T_NUMBER: printf("%.4g", node->u.number); break;
			case T_STRING: printf("\"%s\"", node->u.string); break;
			case T_ARRAY: printf("["); dump_tree(node->child, indent); printf("]"); break;
			case T_OBJECT: printf("{"); dump_tree(node->child, indent+1); printf("}"); break;
			default: fprintf(stderr, "WTF???");
		}
		if (node->next)
			printf(", ");
	}
	printf("\n");
}

/**
 * Return 0 if t1 and t2 trees are the same.
 * Return !0 if they are differ.
 */
int compare_trees(node_t *t1, node_t *t2) {
	if (t1 == NULL && t2 == NULL) {
		return 0;
	}

	if (t1 == t2) {
		return 0;
	}

	return compare_trees(t1->sibling, t2->sibling) || compare_trees(t1->child, t2->child);
}

/************************************************************************************************/

void process_stream(FILE *input) {
		lexem_t l;
		/*while (l = get_next_lexem(input)) {*/
		while (1) {
			l = get_next_lexem(input);
			if (l == L_NUMBER) {
				printf("%s (%f)\n", lexem_to_string[l], g_number);
			} else if (l == L_STRING) {
				printf("%s (\"%s\")\n", lexem_to_string[l], g_string);
			} else if (l == L_END) {
				printf("%s\n", lexem_to_string[l]);
				break;
			} else if (l == L_INVALID) {
				printf("INVALID lexem occured. Stoping...\n");
				break;
			} else {
				printf("%s\n", lexem_to_string[l]);
			}
		}
}

int main(int argc, char *argv[]) {
	node_t *root;

	while (*++argv) {
		if (strcmp(*argv, "-") == 0) {
			/*process_stream(stdin);*/
			root = parse_stream(stdin);
		} else {
			FILE *input = fopen(*argv, "r");
			if (input == NULL) {
				fprintf(stderr, "FAILED TO open file '%s': %s.\n", *argv, strerror(errno));
				continue;
			}

			/*process_stream(input);*/
			root = parse_stream(input);
			fclose(input);
		}
	}

	dump_tree(root, 0);
	return 0;
}
