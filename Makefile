CC     = gcc
CFLAG  = -o -O
INSDIR = /home/yamagen/bin
all: ksf.o
	$(CC) -o ksf ksf.o

ksf: ksf.c
	$(CC) $(CFLAG) ksf.c

clean:
	rm -f *.o

install:
	mv ksf $(INSDIR)
