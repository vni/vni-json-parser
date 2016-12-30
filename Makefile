CC=gcc
CFLAGS=-ggdb3

ALL=json test

all: $(ALL)

json: json.c json.h
	$(CC) $(CFLAGS) -o $@ $<

test: test.c json.c json.h
	$(CC) $(CFLAGS) -o $@ -DENABLE_TESTS $^

clean:
	rm -rf $(ALL)
