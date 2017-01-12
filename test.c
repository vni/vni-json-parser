#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "json.h"

// TODO: test_object_parser: create_node(T_OBJECT, ...) should also be able to accept node tree

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

void test_parser(void) {
	int i;
	int failed_tests = 0;
	node_t *root;
	const char **p;

	struct {
		const char *input;
		type_t expected_type;
		const char *string;
		double number;
	} test_cases[] = {
		{ " 123 ", T_NUMBER, "", 123 },
		{ " true ", T_TRUE, "", 0 },
		{ " false ", T_FALSE, "", 0 },
		{ " null ", T_NULL, "", 0 },
		{ " \"null\" ", T_STRING, "null", 0 },
		{ " \"nu \\n ll\" ", T_STRING, "nu \n ll", 0 },
		{ " \"value: \\u0024. here\" ", T_STRING, "value: $. here", 0 },
		//{ " \"value: \\u0024. here\" ", T_STRING, "value: *. here", 0 }, //INVALID
	};

	for (i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); ++i) {
		p = &test_cases[i].input;
		root = parse_stream(p);

		if (root->type != test_cases[i].expected_type) {
			fprintf(stderr, "root->type (%s) != expected_type (%s).\n", type_to_string[root->type], type_to_string[test_cases[i].expected_type]);
			failed_tests++;
		} else if (root->type == T_NUMBER && root->u.number != test_cases[i].number) {
			fprintf(stderr, "root->type (%s) value (%s) != expected value (%s).\n", type_to_string[root->type], root->u.number, test_cases[i].number);
			failed_tests++;
		} else if (root->type == T_STRING && strcmp(root->u.string, test_cases[i].string) != 0) {
			fprintf(stderr, "root->type (%s) value (%s) != expected value (%s).\n", type_to_string[root->type], root->u.string, test_cases[i].string);
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

void test_parser_array1(void) {
	const char *input = "[1]";
	const char **p = &input;

	node_t *expected_tree;
	expected_tree = create_node(T_ARRAY, 1, create_node(T_NUMBER, 1));

	node_t *root = parse_stream(p);
	if (compare_trees(root, expected_tree)) {
		printf("test_parser_array1. Test FAILED.\n");
	} else {
		printf("test_parser_array1. Test is PASSED.\n");
	}

	free_tree(expected_tree);
	free_tree(root);
}

void test_parser_array2(void) {
	const char *input = "[1  ,           2, \"how are you?\", true, false, null, 10]";
	const char **p = &input;

	node_t *expected_tree = create_node(T_ARRAY, 7,
										create_node(T_NUMBER, 1),
										create_node(T_NUMBER, 2),
										create_node(T_STRING, "how are you?"),
										create_node(T_TRUE),
										create_node(T_FALSE),
										create_node(T_NULL),
										create_node(T_NUMBER, 10.0));;

	node_t *root = parse_stream(p);
	if (compare_trees(root, expected_tree)) {
		printf("test_parser_array2. Test FAILED.\n");
	} else {
		printf("test_parser_array2. Test is PASSED.\n");
	}

	free_tree(expected_tree);
	free_tree(root);
}

void test_parser_array3(void) {
	const char *input = "[1  ,           2, \"how are you?\", true, [true, true, 666], false, null, 10]";
	const char **p = &input;

	node_t *expected_tree = create_node(T_ARRAY, 8,
										create_node(T_NUMBER, 1),
										create_node(T_NUMBER, 2),
										create_node(T_STRING, "how are you?"),
										create_node(T_TRUE),
										create_node(T_ARRAY, 3,
													create_node(T_TRUE),
													create_node(T_TRUE),
													create_node(T_NUMBER, 666.0)
										),
										create_node(T_FALSE),
										create_node(T_NULL),
										create_node(T_NUMBER, 10.0)
							);
	/*
	expected_tree = create_node(T_ARRAY, 0);
	expected_tree->child = create_node(T_NUMBER, 1);
	expected_tree->child->next = create_node(T_NUMBER, 2);
	expected_tree->child->next->next = create_node(T_STRING, "how are you?");
	expected_tree->child->next->next->next = create_node(T_TRUE);
	expected_tree->child->next->next->next->next = create_node(T_ARRAY, 0);
	expected_tree->child->next->next->next->next->child = create_node(T_TRUE);
	expected_tree->child->next->next->next->next->child->next = create_node(T_TRUE);
	expected_tree->child->next->next->next->next->child->next->next = create_node(T_NUMBER, 666);
	expected_tree->child->next->next->next->next->next = create_node(T_FALSE);
	expected_tree->child->next->next->next->next->next->next = create_node(T_NULL);
	expected_tree->child->next->next->next->next->next->next->next = create_node(T_NUMBER, 10);
	*/

	node_t *root = parse_stream(p);
	if (compare_trees(root, expected_tree)) {
		printf("test_parser_array3. Test FAILED.\n");
	} else {
		printf("test_parser_array3. Test is PASSED.\n");
	}

	free_tree(expected_tree);
	free_tree(root);
}

/* simple tests for parser */
void test_parser_array(void) {
	int i;
	int failed_tests = 0;
	node_t *root;
	const char **p;

	struct {
		const char *input;
		type_t expected_type;
		/*node_t *root;*/
	} test_cases[] = {
		{ " [] ", T_ARRAY },
		{ " [1] ", T_ARRAY },
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
		printf("test_parser_array. Failed %d tests.\n", failed_tests);
	} else {
		printf("test_parser_array. All tests are PASSED.\n");
	}
}

void test_parser_object(void) {
	int i;
	int failed_tests = 0;
	node_t *root;
	const char **p;

	struct {
		const char *input;
		type_t expected_type;
	} test_cases[] = {
		{ " {} ", T_OBJECT }
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
		printf("test_parser_object. Failed %d tests.\n", failed_tests);
	} else {
		printf("test_parser_object. All tests are PASSED.\n");
	}
}

void test_parser_object1(void) {
	const char *input = "{\"name\" : \"John Doe\"}";
	const char **p = &input;

	//node_t *expected_root = create_node(T_OBJECT, 0);
	node_t *expected_root = create_node(T_OBJECT, 1,
		                              	"name", create_node(T_STRING, "John Doe"));
	/** unneeded, but let it be an example of how create_node(T_OBJECT, 0) can be used *
	expected_root->child = create_node(T_STRING, "name");
	expected_root->child->child = create_node(T_STRING, "John Doe");
	*/

	node_t *root = parse_stream(p);

	if (compare_trees(expected_root, root)) {
		printf("test_parser_object1. Test FAILED.\n");
	} else {
		printf("test_parser_object1. Test is PASSED.\n");
	}

	free_tree(expected_root);
	free_tree(root);
}

void test_parser_object2(void) {
	const char *input = "{\"name\" : \"John Doe\", \"one\"   : 1111, \"two\"  :\n2222}";
	const char **p = &input;

	node_t *expected_root = create_node(T_OBJECT, 3,
			                            "name", create_node(T_STRING, "John Doe"),
			                            "one", create_node(T_NUMBER, 1111.0),
										"two", create_node(T_NUMBER, 2222.0));
	/** unneeded, but let it be an example of how create_node(T_OBJECT, 0) can be used *
	expected_root->child = create_node(T_STRING, "name");
	expected_root->child->child = create_node(T_STRING, "John Doe");
	expected_root->child->next = create_node(T_STRING, "one");
	expected_root->child->next->child = create_node(T_NUMBER, 1111);
	expected_root->child->next->next = create_node(T_STRING, "two");
	expected_root->child->next->next->child = create_node(T_NUMBER, 2222);
	*/

	node_t *root = parse_stream(p);

	if (compare_trees(expected_root, root)) {
		printf("test_parser_object2. Test FAILED.\n");
	} else {
		printf("test_parser_object2. Test is PASSED.\n");
	}

	free_tree(expected_root);
	free_tree(root);
}

void test_parser_object3(void) {
	const char *input = "{\"name\" : \"John Doe\", \"inner\"  : { \"i1\": 100, \"i2:\":200, \"i3\":{\"ii1\":1111}}}";
	const char **p = &input;

	node_t *expected_root = create_node(T_OBJECT, 2,
			                            "name", create_node(T_STRING, "John Doe"),
										"inner", create_node(T_OBJECT, 3,
											                  "i1", create_node(T_NUMBER, 100.0),
															  "i2", create_node(T_NUMBER, 200.0),
															  "i3", create_node(T_OBJECT, 1,
																                "ii1", create_node(T_NUMBER, 1111.0))));
	/** unneeded, but let it be an example of how create_node(T_OBJECT, 0) can be used *
	node_t *expected_root = create_node(T_OBJECT, 0);
	expected_root->child = create_node(T_STRING, "name");
	expected_root->child->child = create_node(T_STRING, "John Doe");

	expected_root->child->next = create_node(T_STRING, "inner");
	expected_root->child->next->child = create_node(T_OBJECT, 0);

	expected_root->child->next->child->child = create_node(T_STRING, "i1");
	expected_root->child->next->child->child->child = create_node(T_NUMBER, 100);

	expected_root->child->next->child->child->next = create_node(T_STRING, "i2");
	expected_root->child->next->child->child->next->child = create_node(T_NUMBER, 200);

	expected_root->child->next->child->child->next->next = create_node(T_STRING, "i3");
	expected_root->child->next->child->child->next->next->child = create_node(T_OBJECT, 0);
	expected_root->child->next->child->child->next->next->child->child = create_node(T_STRING, "ii1");
	expected_root->child->next->child->child->next->next->child->child->child = create_node(T_NUMBER, 1111);
	*/

	node_t *root = parse_stream(p);

	if (compare_trees(expected_root, root)) {
		printf("test_parser_object3. Test FAILED.\n");
	} else {
		printf("test_parser_object3. Test is PASSED.\n");
	}

	free_tree(expected_root);
	free_tree(root);
}

void test_parser_mixed(void) {
	const char *input = "[\"one\", 2, true, {\"four\":44, \"five\":[1,2,3,4,5], \"six\":[1,2,{},4,[5]]}, [90, 88]]";
	const char **p = &input;

	node_t *expected_root = create_node(T_ARRAY, 5,
			                            create_node(T_STRING, "one"),
										create_node(T_NUMBER, 2.0),
										create_node(T_TRUE),
										create_node(T_OBJECT, 3,
											        "four", create_node(T_NUMBER, 44.0),
													"five", create_node(T_ARRAY, 5,
														                create_node(T_NUMBER, 1.0),
														                create_node(T_NUMBER, 2.0),
														                create_node(T_NUMBER, 3.0),
														                create_node(T_NUMBER, 4.0),
														                create_node(T_NUMBER, 5.0)
													),
													"six", create_node(T_ARRAY, 5,
														               create_node(T_NUMBER, 1.0),
														               create_node(T_NUMBER, 2.0),
														               create_node(T_OBJECT, 0),
														               create_node(T_NUMBER, 4.0),
														               create_node(T_ARRAY, 1,
																				   create_node(T_NUMBER, 5.0)
																	   )
												    )
									    ),
									    create_node(T_ARRAY, 2,
												    create_node(T_NUMBER, 90.0),
												    create_node(T_NUMBER, 88.0)
										)
							);

	node_t *root = parse_stream(p);

	/*
	printf("expected_root:\n"); dump_tree(expected_root, 0);
	printf("root:\n"); dump_tree(root, 0);
	*/

	if (compare_trees(expected_root, root)) {
		printf("test_parser_mixed. Test FAILED.\n");
	} else {
		printf("test_parser_mixed. Test is PASSED.\n");
	}

	free_tree(expected_root);
	free_tree(root);
}

int main(void) {
	/* test lexer */
	printf("Test Lexer:\n");
	test_lexer();
	test_lexer_numbers();
	test_lexer_strings();
	test_lexem_sequence();
	printf("\n");

	/* test parser */
	printf("Test Parser:\n");
	test_parser();
	test_parser_array();
	test_parser_array1();
	test_parser_array2();
	test_parser_array3();
	printf("\n");
	test_parser_object();
	test_parser_object1();
	test_parser_object2();
	test_parser_object3();
	printf("\n");
	test_parser_mixed();
	printf("\n");

	test1();

	return 0;
}
