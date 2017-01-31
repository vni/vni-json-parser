CC=gcc
CFLAGS=-ggdb3 -std=c99

ALL=json test test_output

all: $(ALL)

json: json.c json.h
	$(CC) $(CFLAGS) $< -o json

test: test.c json.c json.h
	$(CC) $(CFLAGS) -DENABLE_TESTS $^ -o test

test_output: test_output.c json.c json.h
	$(CC) $(CFLAGS) -DENABLE_TESTS $^ -o test_output

clean:
	rm -rf $(ALL)
