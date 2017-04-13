#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


int mycat(int inFile, int outFile){
	char buffer[5000];
    ssize_t numBytes;		//data type used to rep size of blocks to be read/written in single op.
    while ((numBytes = read(inFile, buffer, sizeof(buffer))) > 0){
        if (write(outFile, buffer, numBytes) != numBytes)   //we are filling buffer with stuff from file and 
			return -1;										//sending to output
			printf("line\n");
    }
    if(numBytes < 0)
		return (-1);
	else
		return (0);
}

int main(int argc, char **argv){
	if (argc < 2){				//checking for at least one arg
		if (mycat(0, 1) != 0)
	    fprintf(stderr, "\nmycat expects at least one argument...\n");
	    return (-1);
	}
	else{
		for (int i = 1; i < argc; i++){
			int fileDesc = open(argv[i], O_RDONLY);		//open for reading only
			if (fileDesc < 0){ 			//file is not found, lets make one!
				FILE* nFile = fopen(argv[1], "w");
				fclose(nFile);
				fprintf(stderr, "%s does not exist", argv[i]);
				fprintf(stderr, "%s has been created\n", argv[1]);
				return (0);
			}
			else{
				if (mycat(fileDesc, 1) != 0)
					fprintf(stderr, "Error: Unable to access %s\n", argv[i]);
			close(fileDesc);
			}
		}
	}
    return (0);
}
