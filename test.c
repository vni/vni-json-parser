#include "json.h"

L_END ""
L_END "    "
L_END "			 \n		\n "


L_INVALID " asdf ; sadf "
L_INVALID "   -  "


L_COMMA	"	\n  , "
L_COMMA L_INVALID " , asdf  "


L_TRUE " true "
L_TRUE L_INVALID " true True "
L_TRUE L_INVALID " true TRUE "
L_TRUE L_TRUE " true true "


L_FALSE "false"
L_FALSE L_INVALID "false   False"
L_FALSE L_INVALID "false\n	 FALSE"


L_NULL " null"
L_INVALID "nullable"
L_NULL L_INVALID "null nullable"
L_NULL L_INVALID "null Null"
L_NULL L_INVALID "null NULL"

L_NUMBER (1) "1.0"
L_NUMBER (-1) "-1"
L_NUMBER (10.5) "  10.5"
L_NUMBER (-22.2e15) "-22.2E+15"
L_NUMBER (0.123) "123e-3"
L_NUMBER (0) L_INVALID "0 - "

L_STRING (hello "world") "\"hello \\\"world\\\"\""
L_STRING (hello	blah) "hello\tblah"

/* FIXME: add tests for string '\' characters */

int main(void) {
}
