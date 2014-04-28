OBJECT=main.o
CC=g++
main: $(OBJECT)
	$(CC) $(OBJECT) -o main
clean:
	rm -f *.o main
