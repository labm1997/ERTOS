OBJDIR = obj
SRCDIR = src
INCDIR = inc
BINDIR = bin

INC = clock.h task.h task_fifo.h

OBJ = clock.o task_fifo.o

OBJECTS=$(patsubst %,$(OBJDIR)/%,$(OBJ))
HEADERS=$(patsubst %,$(INCDIR)/%,$(INC))

$(BINDIR)/ertos: $(OBJECTS) $(SRCDIR)/ertos.c | $(BINDIR)
	msp430-gcc -mmcu=msp430f5529 -g -o $@ $(SRCDIR)/ertos.c $(OBJECTS) -I $(INCDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	msp430-gcc -mmcu=msp430f5529 -g -c -o $@ $< -I $(INCDIR)

$(BINDIR):
	mkdir $(BINDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm $(OBJDIR) $(BINDIR) -Rf
