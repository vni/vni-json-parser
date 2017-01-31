/*
 * This file tests the output functions of the json lib.
 * It creates json structure by hands and compare the
 * result of the output function with expected test result.
 */

#include <stdio.h>
#include "json.h"

void test1(void) {
	node_t *root = create_node(T_NULL);
	printf("test1():\n");
	dump_out_stream(root);
	printf("\n");
}

void test2(void) {
	node_t *root = create_node(T_FALSE);
	printf("test2():\n");
	dump_out_stream(root);
	printf("\n");
}

void test3(void) {
	node_t *root = create_node(T_TRUE);
	printf("test3():\n");
	dump_out_stream(root);
	printf("\n");
}

void test4(void) {
	node_t *root = create_node(T_NUMBER, 4.18e2);
	printf("test4():\n");
	dump_out_stream(root);
	printf("\n");
}

void test5(void) {
	node_t *root = create_node(T_STRING, "hello, miserable, world!");
	printf("test5():\n");
	dump_out_stream(root);
	printf("\n");
}

void test6(void) {
	node_t *root = create_node(T_ARRAY, 4,
							   create_node(T_NUMBER, 1.0),
							   create_node(T_STRING, "two"),
							   create_node(T_TRUE),
							   create_node(T_NULL));
	printf("test6():\n");
	dump_out_stream(root);
	printf("\n");
}

void test7(void) {
	node_t *root = create_node(T_OBJECT, 5,
			                   "one", create_node(T_NUMBER, 1.0),
							   "two", create_node(T_TRUE),
							   "three", create_node(T_ARRAY, 3,
								                    create_node(T_STRING, "zero"),
													create_node(T_STRING, "one"),
													create_node(T_STRING, "two")
												    ),
							   "four", create_node(T_OBJECT, 2,
								                   "address", create_node(T_STRING, "Zhukova 47a"),
												   "age", create_node(T_NUMBER, 27.0)
												   ),
							   "five", create_node(T_ARRAY, 3,
								                   create_node(T_OBJECT, 2,
													           "name", create_node(T_STRING, "Mykola"),
															   "sex", create_node(T_STRING, "M")),
								                   create_node(T_OBJECT, 2,
													           "name", create_node(T_STRING, "Olenjka"),
															   "sex", create_node(T_STRING, "F")),
								                   create_node(T_OBJECT, 2,
													           "name", create_node(T_STRING, "Beljchenochek"),
															   "sex", create_node(T_STRING, "F"))
												   )
			                   );
	printf("test7():\n");
	dump_out_stream(root);
	printf("\n");
}

int main(void) {
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();

	return 0;
}
