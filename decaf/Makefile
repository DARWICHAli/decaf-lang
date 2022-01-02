CC ?= gcc
CFLAGS ?=-std=c11 -Wpedantic -Wall -Wextra
LDFLAGS ?=
BISONFLAGS ?= -t
FLEXFLAGS ?= -s

.PHONY: clean test decaf

SRC_DIR=src
SRC_TS_DIR=test/test_suite/src
INCLUDE_DIR=include
INCLUDE_TS_DIR=test/test_suite/include
BIN_DIR=bin
OBJ_DIR=obj
OBJ_TS_DIR=obj/ts
TEST_DIR=test
UNIT_TEST_DIR=$(TEST_DIR)/unitary
INTEGRATION_TEST_DIR=$(TEST_DIR)/integration

OBJ_MAIN=decaf.o
OBJS_STAT=decaf.tab.o decaf_lex.o
OBJS_DYN=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))
OBJS=$(patsubst %,$(OBJ_DIR)/%,$(OBJS_STAT)) $(OBJS_DYN)
OBJS_TS=$(patsubst $(SRC_TS_DIR)/%.c, $(OBJ_TS_DIR)/%.o, $(wildcard $(SRC_TS_DIR)/*.c))

decaf: $(BIN_DIR)/decaf

$(BIN_DIR)/decaf: $(OBJS) | $(BIN_DIR)
	$(CC) -I$(INCLUDE_DIR) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -I$(INCLUDE_DIR) $(CFLAGS) -o $@ -c $<

$(SRC_DIR)/decaf_lex.c: $(SRC_DIR)/decaf.l
	flex $(FLEXFLAGS) -o $@ $<

$(SRC_DIR)/decaf.tab.c: $(SRC_DIR)/decaf.y
	bison $(BISONFLAGS) --defines=$(patsubst $(SRC_DIR)/%.c,$(INCLUDE_DIR)/%.h,$@) --output=$@ $<

# Tests
test: $(patsubst $(UNIT_TEST_DIR)/%.c,$(BIN_DIR)/$(TEST_DIR)/%,$(wildcard $(UNIT_TEST_DIR)/*.c)) decaf | $(BIN_DIR)/$(TEST_DIR)
	bash $(UNIT_TEST_DIR)/launch.sh $(BIN_DIR)/$(TEST_DIR) && bash $(INTEGRATION_TEST_DIR)/launch.sh $(BIN_DIR)

$(BIN_DIR)/$(TEST_DIR):
	mkdir -p $@

$(OBJ_DIR)/$(TEST_DIR):
	mkdir -p $@

$(OBJ_TS_DIR):
	mkdir -p $@

$(BIN_DIR)/$(TEST_DIR)/%: $(patsubst %,$(OBJ_DIR)/$(TEST_DIR)/%.o,%) $(filter-out %$(OBJ_MAIN), $(OBJS)) $(OBJS_TS) | $(BIN_DIR)/$(TEST_DIR)
	$(CC) -I$(INCLUDE_DIR) -I$(INCLUDE_TS_DIR) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/$(TEST_DIR)/%.o: $(UNIT_TEST_DIR)/%.c $(OBJS_TS) | $(OBJ_DIR)/$(TEST_DIR)
	$(CC) -I$(INCLUDE_DIR) -I$(INCLUDE_TS_DIR) $(CFLAGS) -o $@ -c $<

$(OBJ_TS_DIR)/%.o: $(SRC_TS_DIR)/%.c | $(OBJ_TS_DIR)
	$(CC) -I$(INCLUDE_DIR) -I$(INCLUDE_TS_DIR) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf bin/ obj/ $(SRC_DIR)/decaf_lex.c $(SRC_DIR)/decaf.tab.c $(INCLUDE_DIR)/decaf.tab.h html/ latex *.gcno *.gcda */.gcno */.gcda */*.gcno */*.gcda
