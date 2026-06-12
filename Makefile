EXAMPLES= \
      examples/00_hello.out \
      examples/05_nested_loops.out \
      examples/10_fibonacci.out \
      examples/20_questions.out \
      examples/30_adventure.out \
      examples/40_better_jumping.out \
      examples/50_tictactoe.out

.PHONY: examples
examples: $(EXAMPLES)

awass: awass.c
	$(CC) -ggdb -Wall -Wextra -pedantic -o awass awass.c

%.out: %.awa awass
	./awass -o $@ $<
