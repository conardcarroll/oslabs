CC = gcc
CFLAGS = -g -Wall -I./

TESTS = tests/one_to_many \
		tests/many_to_one \
		tests/many_to_many \
		tests/consume_non_threaded \
		tests/no_connections \
		tests/consumers_at_different_rates \
		tests/disconnect_reconnect

TESTS_C = ${TESTS:=.c}

.PHONY: gui

all: main

main: main.c streams.c streams.h
	$(CC) $(CFLAGS) $(LIBS) main.c streams.c -o $@ -lpthread

gui: gui/gui.c streams.c streams.h
	$(CC) $(CFLAGS) $(LIBS) gui/gui.c streams.c `pkg-config --libs --cflags gtk+-2.0` -o gui/gui -lpthread

tests: ${TESTS} 

$(TESTS): ${TESTS_C} streams.c streams.h
	@echo
	@echo "========== "$@" =========="
	@echo
	$(CC) $(CFLAGS) $(LIBS) $@.c streams.c -o $@ -lpthread

clean:
	rm ${TESTS} main
