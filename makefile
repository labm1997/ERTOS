OBJDIR = obj
SRCDIR = src
INCDIR = inc
BINDIR = bin

INC = clock.h task.h task_fifo.h

OBJ = clock.o task_fifo.o
SRC = clock.c ertos.c task_fifo.c

OBJECTS=$(patsubst %,$(OBJDIR)/%,$(OBJ))
HEADERS=$(patsubst %,$(INCDIR)/%,$(INC))

$(BINDIR)/ertos: $(OBJECTS)
	msp430-gcc -mmcu=msp430f5529 -g -o $@ $^ -I $(INCDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	msp430-gcc -mmcu=msp430f5529 -g -c -o $@ $< -I $(INCDIR)

clean:
	rm $(OBJDIR)/*.o
	rm $(BINDIR)/ertos
