#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "json.h"

// TODO: test parser. generate the tree from the input string and test it agains generated one

void test_lexer_numbers(void) {
	int i;
	int failed_tests = 0;
	lexem_t l;

	struct test_case {
		const char *input;
		lexem_t expected; /* the first lexem in input */
		double number;
	} test_cases[] = {
		{ "  0", L_NUMBER, 0 },
		{ " \t1.0", L_NUMBER, 1 },
		{ " \n-1", L_NUMBER, -1 },
		{ "   10.5", L_NUMBER, 10.5 },
		{ "\n 22.2E+15", L_NUMBER, 22.2E+15 },
		{ "-22.2E+15", L_NUMBER, -22.2E+15 },
		{ "-1.23e+4", L_NUMBER, -1.23e+4 },
		{ "\t 123E-2", L_NUMBER, 1.23 },
		{ "\t 123e-3", L_NUMBER, 0.123 },
		// { "2.3", L_NUMBER, 22.3},
		{ "\t\n\t22.3", L_NUMBER, 22.3}
	};

	for (i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); ++i) {
		const char **p = &test_cases[i].input;
		l = get_next_lexem(p);
		if (l != test_cases[i].expected || g_number != test_cases[i].number) {
			fprintf(stderr, "test_lexer_numbers failed. Test: %d. test input: \"%s\".\n", i, test_cases[i].input);
			fprintf(stderr, "Expected lexem: '%s', but got: '%s'.\n", lexem_to_string[test_cases[i].expected], lexem_to_string[l]);
			fprintf(stderr, "Expected number: %g, but got: %g.\n", i, test_cases[i].number, g_number);
			failed_tests++;
		}

		l = get_next_lexem(p);
		if (l != L_END) {
			fprintf(stderr, "test_lexer_numbers failed. Test: %d. Expected to get L_END as next lexem in test string: \"%s\", but got: %s\n",
					i, test_cases[i].input, lexem_to_string[l]);
		}
	}

	if (failed_tests) {
		printf("test_lexer_numbers. Failed %d test%s.\n", failed_tests, failed_tests > 1 ? "s" : "");
	} else {
		printf("test_lexer_numbers. All tests are PASSED.\n");
	}
}

void test_lexer_strings(void) {
	int i;
	int failed_tests = 0;
	lexem_t l;

	struct test_case {
		const char *input;
		lexem_t expected; /* the first lexem in input */
		const char *string;
	} test_cases[] = {
		{ "\"\"", L_STRING, "" },
		{ " \"\" ", L_STRING, "" },
		{ " \" \" ", L_STRING, " " },
		{ "  \t\n\t   \"\" ", L_STRING, "" },
		{ "  \"hello world\" ", L_STRING, "hello world" },
		{ "  \"hello\\nworld\" ", L_STRING, "hello\nworld" },
		{ "  \"hello\\tworld\" ", L_STRING, "hello\tworld" },
		{ "  \"\\thello\\fworld\" ", L_STRING, "\thello\fworld" },
		{ "\"\\u0024\"", L_STRING, "$" },
		{ "\t \"\\u0024\"\t ", L_STRING, "$" },
		{ "\n \"\\u0026\"\t ", L_STRING, "&" },
		{ "\n \"\\u00A9\"\t ", L_STRING, "\xC2\xA9" },
		{ "\n \"\\u00a9\"\t ", L_STRING, "\xC2\xA9" },
		{ "\n \"\\u2260\"\t ", L_STRING, "\xE2\x89\xA0" },
		{ "\n \"\\u22\"\t ", L_INVALID_STRING, "" },
		{ "\n \"\\u2\0\"\t ", L_INVALID_STRING, "" },
		{ "\n \"\\u\0\"\t ", L_INVALID_STRING, "" },
		{ "\n \"\\\0\"\t ", L_INVALID_STRING, "" },
	};

	for (i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); ++i) {
		const char *p = test_cases[i].input;
		l = get_next_lexem(&p);
		if (l != test_cases[i].expected) {
			fprintf(stderr, "test_lexer_strings failed. Test: %d. test input: \"%s\".\n", i, test_cases[i].input);
			fprintf(stderr, "Expected lexem: %s, got: '%s'.\n", lexem_to_string[test_cases[i].expected], lexem_to_string[l]);
			fprintf(stderr, "Expected string: \"%s\", got: \"%s\".\n", test_cases[i].string, g_string);
			failed_tests++;
		} else if (l != L_INVALID_STRING && strcmp(g_string, test_cases[i].string) != 0) {
			fprintf(stderr, "test_lexer_strings failed. Test: %d. test input: \"%s\".\n", i, test_cases[i].input);
			fprintf(stderr, "Expected string: \"%s\", got: \"%s\".\n", test_cases[i].string, g_string);
			failed_tests++;
		}

		if (l != L_INVALID_STRING) {
			l = get_next_lexem(&p);
			if (l != L_END) {
				fprintf(stderr, "test_lexer_strings failed. Test: %d. Expected to get L_END as next lexem in test string: \"%s\", but got: %s\n",
						i, test_cases[i].input, lexem_to_string[l]);
				fprintf(stderr, "p: '%s'\n", p);
			}
		}
	}

	if (failed_tests) {
		printf("test_lexer_strings. Failed %d test%s.\n", failed_tests, failed_tests > 1 ? "s" : "");
	} else {
		printf("test_lexer_strings. All tests are PASSED.\n");
	}
}

void test_lexer(void) {
	int i;
	int failed_tests = 0;
	lexem_t l;

	struct test_case {
		const char *input;
		lexem_t expected; /* the first lexem in input */
	} test_cases[] = {
		/* empty strings */
		{ "", L_END },

		/* white spaces */
		{ "   ", L_END },
		{ " \t \t \t \n \n \t \n   ", L_END },
		{ " \n	 \n ", L_END },

		/* invalid strings */
		{ "         - ", L_INVALID },
		{ " asfd ; asd", L_INVALID },


		/* comma */
		{ ",", L_COMMA },
		{ "    ,   ", L_COMMA },
		{ "  \t  \n  , \n \t", L_COMMA },

		{ ":", L_COLON },
		{ "  : ", L_COLON },
		{ "\t\n\t:", L_COLON },

		/* bracket */
		{ "  [ ", L_LBRACKET },
		{ "  ] ", L_RBRACKET },

		/* braces */
		{ "{", L_LBRACE },
		{ "}", L_RBRACE },

		{ "null", L_NULL },
		{ "nullable", L_INVALID },
		{ "Null", L_INVALID },
		{ "nil", L_INVALID },
		{ "NULL", L_INVALID },

		/* false */
		{ "\nfalse", L_FALSE },
		{ "False", L_INVALID },
		{ "FALSE", L_INVALID },
		{ "falSE", L_INVALID },
		{ "falseable", L_INVALID },

		/* true */
		{ "\n\ntrue   ", L_TRUE },
		{ "   True    ", L_INVALID },
		{ "   TRUE    ", L_INVALID },
		{ "   TrUe    ", L_INVALID },
		{ "\n tRUe    ", L_INVALID },
		{ "\n trueable", L_INVALID },
	};

	for (i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); ++i) {
		const char **p = &test_cases[i].input;
		l = get_next_lexem(p);
		if (l != test_cases[i].expected) {
			fprintf(stderr, "test_lexer failed. Test: %d. Expected lexem: '%s', but got: '%s'.\n", i, lexem_to_string[test_cases[i].expected], lexem_to_string[l]);
			failed_tests++;
		}
	}

	char *input = "";
	char **p = &input;

	l = get_next_lexem((const char **)p);

	assert(l == L_END);

	if (failed_tests) {
		printf("test_lexer. Failed %d tests.\n", failed_tests);
	} else {
		printf("test_lexer. All tests are PASSED.\n");
	}
}

/**
 * @return 0 on success, 1 on failure.
 *
 * @note. Maybe segfault here, cause the user do not pass a valid lexem sequence terminating with L_END
 */
int compare_sequence(const char *input_string, ...) {
	const char *p = input_string;
	lexem_t lex1, lex2;
	va_list args;

	va_start(args, input_string);

	while (1) {
		lex1 = get_next_lexem(&p);
		lex2 = va_arg(args, lexem_t);

		if (lex1 != lex2) {
			fprintf(stderr, "compare_sequence. input_string: \"%s\", lex1: %s, lex2: %s\n", input_string, lexem_to_string[lex1], lexem_to_string[lex2]);
			return 1;
		}

		if (lex1 == L_END) {
			break;
		}
	}
	va_end(args);
	return 0;
}

void test_lexem_sequence(void) {
	int failed_tests = 0;

	failed_tests += compare_sequence(
		"1 2 3 4 14.5e6 \"hello\t\\u2617\", {\"key\": [ 2, 3, 5 ]}",
		L_NUMBER, L_NUMBER, L_NUMBER, L_NUMBER, L_NUMBER, L_STRING, L_COMMA, L_LBRACE, L_STRING,
		L_COLON, L_LBRACKET, L_NUMBER, L_COMMA, L_NUMBER, L_COMMA, L_NUMBER, L_RBRACKET, L_RBRACE,
		L_END);

	failed_tests += compare_sequence(
		"[ 10, 20, 30, true, false, null, \"one\", \"two\", \"three\" ]",
		L_LBRACKET, L_NUMBER, L_COMMA, L_NUMBER, L_COMMA, L_NUMBER, L_COMMA, L_TRUE, L_COMMA, L_FALSE,
		L_COMMA, L_NULL, L_COMMA, L_STRING, L_COMMA, L_STRING, L_COMMA, L_STRING, L_RBRACKET, L_END);

	if (failed_tests) {
		printf("test_lexem_sequence. failed_tests: %d\n", failed_tests);
	} else {
		printf("test_lexem_sequence. All tests are PASSED.\n");
	}
}

void test1(void) {
	node_t *root;

	char *input = "10";
	const char **p = (const char **)&input;
	root = parse_stream(p);

	if (root->type != T_NUMBER) {
		fprintf(stderr, "root->type != T_NUMBER, root->type: %s\n", type_to_string[root->type]);
		exit(EXIT_FAILURE);
	}

	free_tree(root);
}

/* simple tests for parser */
void test_parser(void) {
	int i;
	int failed_tests = 0;
	node_t *root;
	const char **p;

	struct {
		const char *input;
		type_t expected_type;
		union {
			const char *str;
			double num;
		} u;
	} test_cases[] = {
		{ " 123 ", T_NUMBER, 123 },
		{ " true ", T_TRUE, 0 },
		{ " false ", T_FALSE, 0 },
		{ " null ", T_NULL, 0 },
		{ " [] ", T_ARRAY, 0 },
		{ " {} ", T_OBJECT, 0 }
	};

	for (i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); ++i) {
		p = &test_cases[i].input;
		root = parse_stream(p);

		if (root->type != test_cases[i].expected_type) {
			fprintf(stderr, "root->type (%s) != expected_type (%s).\n", type_to_string[root->type], type_to_string[test_cases[i].expected_type]);
			failed_tests++;
		}

		free_tree(root);
	}

	if (failed_tests) {
		printf("test_parser. Failed %d tests.\n", failed_tests);
	} else {
		printf("test_parser. All tests are PASSED.\n");
	}
}

int main(void) {
	/* test lexer */
	test_lexer();
	test_lexer_numbers();
	test_lexer_strings();
	test_lexem_sequence();

	/* test parser */
	test_parser();

	test1();

	return 0;
}
