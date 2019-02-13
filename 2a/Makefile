default: wish.o func.o
	gcc wish.o func.o -o wish
wish.o: wish.c
	gcc -Wall -Werror -c wish.c
func.o: func.c
	gcc -Wall -Werror -c func.c
clean:
	rm *.o
	rm wish
