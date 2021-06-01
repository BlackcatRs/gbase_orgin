CC = gcc
CCFLAGS = `gtk-config --cflags`
LDFLAGS = `gtk-config --libs`

gbase:	gbase.o
	$(CC) gbase.o $(LDFLAGS) -o gbase

clean:
	rm -f gbase gbase.o gbase.c~

# Make object files:
%.o:
	$(CC) $(CCFLAGS) -c $*.c

gbase.o: gbase.c