EXAMPLES= \
      examples/00_fibonacci.out \
      examples/10_questions.out \
      examples/20_adventure.out \
      examples/30_better_jumping.out

.PHONY: examples
examples: $(EXAMPLES)

awass: awass.c
	$(CC) -Wall -Wextra -pedantic -o awass awass.c

%.out: %.awa awass
	./awass -o $@ $<
