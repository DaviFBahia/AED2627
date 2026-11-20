# ============================================================================
# Makefile for AED 2026/2027 1st Project: VectorScene ADT
# ============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c17 -g -O2
LDFLAGS = -lm

# Test suite target
TEST_SRC = unit_tests.c primitive.c vector_scene.c
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_TARGET = unit_tests

# Demo application target
DEMO_SRC = demo_app.c primitive.c vector_scene.c
DEMO_OBJ = $(DEMO_SRC:.c=.o)
DEMO_TARGET = demo_app

# Testes proprios do grupo (casos limite, estabilidade, contadores)
MEUS_SRC = meus_testes.c primitive.c vector_scene.c
MEUS_OBJ = $(MEUS_SRC:.c=.o)
MEUS_TARGET = meus_testes

# Benchmark (ordenacao e picking): ./benchmark sort | ./benchmark pick
BENCH_SRC = benchmark.c primitive.c vector_scene.c
BENCH_OBJ = $(BENCH_SRC:.c=.o)
BENCH_TARGET = benchmark

all: $(TEST_TARGET) $(DEMO_TARGET) $(BENCH_TARGET) $(MEUS_TARGET)

$(TEST_TARGET): $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(DEMO_TARGET): $(DEMO_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BENCH_TARGET): $(BENCH_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(MEUS_TARGET): $(MEUS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c primitive.h primitive_internal.h vector_scene.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

test-fase1: $(TEST_TARGET)
	./$(TEST_TARGET) 1

test-fase2: $(TEST_TARGET)
	./$(TEST_TARGET) 2

test-fase3: $(TEST_TARGET)
	./$(TEST_TARGET) 3

test-fase4: $(TEST_TARGET)
	./$(TEST_TARGET) 4

test-fase5: $(TEST_TARGET)
	./$(TEST_TARGET) 5

meus-testes: $(MEUS_TARGET)
	./$(MEUS_TARGET)

demo: $(DEMO_TARGET)
	./$(DEMO_TARGET)

memcheck: $(TEST_TARGET) $(MEUS_TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=99 ./$(TEST_TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=99 ./$(MEUS_TARGET)

clean:
	rm -f *.o $(TEST_TARGET) $(DEMO_TARGET) $(BENCH_TARGET) $(MEUS_TARGET)

.PHONY: all test test-fase1 test-fase2 test-fase3 test-fase4 test-fase5 meus-testes demo memcheck clean
