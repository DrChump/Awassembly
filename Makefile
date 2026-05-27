EXAMPLES= \
      examples/10questions.out \
      examples/fibonacci.out \
      examples/adventure.out

.PHONY: examples
examples: $(EXAMPLES)

awass: awass.c
	$(CC) -Wall -Wextra -pedantic -o awass awass.c

%.out: %.awa awass
	./awass -o $@ $<
