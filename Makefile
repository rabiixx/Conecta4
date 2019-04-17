all: cliente1.c servidor1.c servidor2.c cliente3.c servidor3.c
	gcc cliente1.c -o cliente1 -Wall -pedantic
	gcc servidor1.c -o servidor1 -Wall -pedantic
	gcc servidor2.c -o servidor2 -Wall -pedantic
	gcc cliente3.c -o cliente3 -Wall -pedantic	
	gcc servidor3.c -o servidor3 -Wall -pedantic

objetivo1: cliente1.c servidor1.c
	gcc cliente1.c -o cliente1 -Wall -pedantic
	gcc servidor1.c -o servidor1 -Wall -pedantic

objetivo2: cliente1.c servidor1.c
	gcc servidor2.c -o servidor2 -Wall -pedantic

objetivo3: cliente3.c servidor3.c
	gcc cliente3.c -o cliente3 -Wall -pedantic
	gcc servidor3.c -o servidor3 -Wall -pedantic

clean:
	@- rm -v servidor1
	@- rm -v cliente1
	@- rm -v servidor2
	@- rm -v cliente3
	@- rm -v servidor3
