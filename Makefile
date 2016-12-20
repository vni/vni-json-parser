ALL=json test

all: $(ALL)

json: json.c json.h
	gcc -o $@ $<

#test: test.c json.h
#	gcc -o $@ $<

clean:
	rm -rf $(ALL)
