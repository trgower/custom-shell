all: mysh mycp myls mycat

mysh: mysh.c
	gcc -o mysh mysh.c

mycp: mycp.c
	gcc -o mycp mycp.c

myls: myls.c
	gcc -o myls myls.c

mycat: mycat.c
	gcc -o mycat mycat.c

clean:
	rm -rf mysh mycp myls mycat

