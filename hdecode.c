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
# include "hshare.c"

# define SIZE 256
# define BIG_STREAM 4096
# define SAFE 100

/* This will read the header from input 
 * encoded file and generate histogram*/
Node **headerToHistogram(int fd_in, int *oneChar){
        Node **histogram = malloc(SIZE*sizeof(Node *));
        int count;
	uint8_t *num_1 = malloc(1), *currentChar = malloc(1);
	uint32_t *currentFreq = malloc(4);

	alloc_check(histogram);
	alloc_check(num_1);
	alloc_check(currentChar);
	alloc_check(currentFreq);
	
	 /* Initializing histogram*/
        for (count = 0; count < SIZE; count ++){
                Node *huffNode = malloc(2*sizeof(int) + 2*sizeof(Node *));
		alloc_check(huffNode);
		huffNode -> Char = count;
                huffNode -> freq = 0;
                huffNode -> left = NULL;
                huffNode -> right = NULL;
                histogram[count] = huffNode;
        }

        /* Reading and filling up histogram*/
        io_check(read(fd_in, num_1, 1));

	if (*num_1 == 0){
		*oneChar = 1;	
	}	
	/*Let the reading begin*/ 
	for (count = 0; count < *num_1 + 1; count++){
		int charr, freqq;
		io_check(read(fd_in, currentChar, 1));
		io_check(read(fd_in, currentFreq, 4));
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


void writeOutput2(int fd_in, int fd_out, Node *bst, long int amountChar){

        uint8_t *current;
        uint8_t mask;
        int charWritten = 0;
        Node *node = bst; /*Start at root*/
        uint8_t *buff;
	int byteCount;
	int capCount = 1;
	current = malloc(BIG_STREAM);
        buff = malloc(BIG_STREAM);
        alloc_check(current);
        alloc_check(buff);
	
        while (  read(fd_in, current, BIG_STREAM) != 0){
		int count = 0;
	        byteCount = 0;
		while (count < BIG_STREAM){
		       if (byteCount > capCount*BIG_STREAM - SAFE){
			capCount++;
			buff = realloc(buff, capCount * BIG_STREAM);
			}					

	               for ( mask = 0x80; mask; mask>>= 1){

                        /*If at a leaf ...*/
                        if (node -> right == NULL &&  node -> left == NULL){
                                charWritten++;
                                *(buff + byteCount) = (uint8_t) node -> Char;
				byteCount++;
                                /*Restart at the root*/
                                node = bst;
                        }
                        if (charWritten == amountChar){
                                io_check(write(fd_out, buff, byteCount));
				free(buff);
				free(current);
				return;	
			}
                        /*Go right*/
                        if (mask & *(current + count)){
                                node = node -> right;

                        /*Go left*/
                        }else{
                                node = node -> left;
                        }
                	}
			count++;
		}
		io_check(write(fd_out, buff, byteCount));
        }
}




void writeOneChar(Node **histogram, int fd_out){

	uint8_t *oneChar;
	int freq, count;
	uint8_t *buffer;
	oneChar = malloc(1);
 	alloc_check(oneChar);	
	for (count = 0; count < SIZE; count++){
	
		if (histogram[count] -> freq != 0){
		*oneChar = count;
		freq = histogram[count] -> freq;
		break;
		}	
	}
	buffer = malloc(freq);
	for (count = 0; count < freq; count++){
		buffer[count] = *oneChar;
	}
	if (write(fd_out, buffer, freq) != freq){
	perror("write out");
	exit(1);
	}
	free(oneChar);
}

int main(int argc, char *argv[]){
	
	int fd_in, fd_out, count;
	char emptyTest[1];
	Node **histogram;
	LLNode *ll;
        Node *bst;
	int *oneChar = malloc(sizeof(int*));
	long int amountChar;
	alloc_check(oneChar);
	/*Parsing command line args*/
	if (argc == 1 || (argc > 1 && strcmp(argv[1], "-") == 0)){
		fd_in = STDIN_FILENO;
		fd_out = STDOUT_FILENO;
	}else if (argc == 2){
		fd_in = open(argv[1], O_RDONLY);
                fd_out = STDOUT_FILENO;
	}else{
		if (strcmp(argv[1], "<")){
		fd_in = open(argv[1], O_RDONLY);
		fd_out = open(argv[2], O_RDWR | O_CREAT | O_TRUNC,
                 (S_IWUSR | S_IXUSR| S_IRUSR | S_IROTH | S_IXOTH | S_IWOTH) );
		}else{
			fd_in = STDIN_FILENO;
                	fd_out = STDOUT_FILENO;
		}

	}
		
	/*Is it empty?*/
	if (read(fd_in, emptyTest, 1) != 1) {
		return 0;
	}else{
	/*Otherwhise, let's reset our pointer*/
		lseek(fd_in, 0, SEEK_SET);	
	}
	*oneChar = 0;
	histogram = headerToHistogram(fd_in, oneChar);
	/*One Char File*/
	if (*oneChar == 1){
		writeOneChar(histogram, fd_out);
		free(histogram);	
		exit(0);
	}
	ll = createLL(histogram);
        bst = createBST(ll);
	amountChar = bst -> freq;
	
	writeOutput2(fd_in, fd_out, bst, amountChar); 
		
	 /* Deallocating stuff*/
        close(fd_in);
        close(fd_out);

        for (count = 0; count < SIZE; count++){
                free(histogram[count]);
         }
        free(histogram);
        free(bst);
        free(ll);

	return 0;
}
