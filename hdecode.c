# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
# include <errno.h>
# include <stdint.h>
# include <arpa/inet.h>

# include "htable.h"

# define SIZE 256


/* This will read the header from input 
 * encoded file and generate histogram*/
Node **headerToHistogram(int fd_in){
        Node **histogram = malloc(SIZE*sizeof(Node *));
        int count;
	uint8_t *num_1 = malloc(1), *currentChar = malloc(1);
	uint32_t *currentFreq = malloc(4);

	 /* Initializing histogram*/
        for (count = 0; count < SIZE; count ++){
                Node *huffNode = malloc(2*sizeof(int) + 2*sizeof(Node *));
                huffNode -> Char = count;
                huffNode -> freq = 0;
                huffNode -> left = NULL;
                huffNode -> right = NULL;
                histogram[count] = huffNode;
        }

        /* Reading and filling up histogram*/
        read(fd_in, num_1, 1);
	
	/*Let the reading begin*/ 

	for (count = 0; count < *num_1 + 1; count++){
		int charr, freqq;
		read(fd_in, currentChar, 1);
		read(fd_in, currentFreq, 4);
		/*Converting back to host byte order*/
		*currentFreq = ntohl(*currentFreq);
		charr = (int) *currentChar;
		freqq = (int) *currentFreq;
		histogram[charr] -> freq = freqq; 			
	}
	free(num_1); 
	free(currentChar);
	free(currentFreq);
	return histogram;
}


int main(int argc, char *argv[]){
	
	int fd_in, fd_out, count;
	char emptyTest[1];
	Node **histogram;

	/*Parsing command line args*/
	if (argc == 1){
		fd_in = STDIN_FILENO;
		fd_out = STDOUT_FILENO;
	}else if (argc == 2){
		fd_in = open(argv[1], O_RDONLY);
                fd_out = STDOUT_FILENO;
	}else{
		fd_in = open(argv[1], O_RDONLY);
		fd_out = open(argv[2], O_RDWR | O_CREAT | O_TRUNC,
                 (S_IWUSR | S_IXUSR| S_IRUSR | S_IROTH | S_IXOTH | S_IWOTH) );
	}
		
	/*Is it empty?*/
	if (read(fd_in, emptyTest, 1) != 1) {
		return 0;
	}else{
	/*Otherwhise, let's reset out pointer*/
		lseek(fd_in, 0, SEEK_SET);	
	}

	histogram = headerToHistogram(fd_in);


	 for (count = 0; count < SIZE; count++){
                if (histogram[count] -> freq != 0){
                printf("char: %d freq: %d\n", histogram[count] -> Char, 
		histogram[count] -> freq);
                                
                }      

         } 

	return 0;
}
