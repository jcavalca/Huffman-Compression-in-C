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
# define CODE_SIZE 256
# define CHUNCK 500
# define SAFE_MARGIN 100
# define BIG_STREAM 4096


Node **build_histogram(int fd, int *emptyFlag, uint32_t *freqArr, int *oF){
        Node **histogram;
        int currentChar;
        int count, onec;
        uint8_t *in = malloc(BIG_STREAM);
        histogram = malloc(SIZE*sizeof(Node *));
        alloc_check(in);
        alloc_check(histogram);

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
        currentChar = read(fd, in, BIG_STREAM);
   	io_check(currentChar);
        while (currentChar != 0){
		int byteCount = 0;
		while (byteCount < currentChar){
                (histogram[(int) in[byteCount]] -> freq)++;
                count++;
		byteCount++;
		}
		currentChar = read(fd, in, BIG_STREAM);	
      		io_check(currentChar);
	}
        onec = 0;
        for (count = 0; count < SIZE; count++){
                if(histogram[count] -> freq != 0){
                        uint32_t temp = (uint32_t) histogram[count] -> freq;
                        freqArr[count] = temp;
                        onec++;
                }
        }

        /* Empty file*/
        if (onec == 0){
              *emptyFlag = 1;}
        else
              *emptyFlag = 0;

        if (onec == 1){
                *oF = 1;}
        else{
                *oF = 0;}
        return histogram;
}

/*Created a strcat*/
char *mystrcat(char *word1, char *word2){
	int new_len = strlen(word1) + strlen(word2) + 1;
	int count;
	int count2 = 0;
	int len1 = strlen(word1);
	char *new_word = malloc( new_len * sizeof(char));
	alloc_check(new_word);
	
	for (count = 0; count < new_len - 1; count++){
		if (count < len1){
			new_word[count] = word1[count];
		}else{
			new_word[count] = word2[count2];
			count2++;
		}
	}
	new_word[count] = '\0';
	return new_word;
}
/* Puts each char encoding as a string
 * in an array to be returned, puts NULL if 
 * char is not present.  */

char **getEncoding(Node *node, char **codeArr, char *code){
	char *new_code;

	if (node -> left == NULL && node -> right == NULL){
		codeArr[node -> Char] = code;
	}
	if (node -> left != NULL){
		new_code = mystrcat(code, "0");
		if (node -> right == NULL){free(code);}
		getEncoding(node -> left, codeArr, new_code);
	}
	 if (node -> right != NULL){
		new_code = mystrcat(code, "1");		 
		getEncoding(node -> right, codeArr, new_code);
	 }
	return codeArr;
}

char *myrealloc(char *old, int new_cap){

	int count = 0, count2;

	char *new = malloc(new_cap);
	alloc_check(new);
	
	while (old[count] != '\0'){
	new[count] = old[count];
	count++;
	}
	for (count2 = count; count2 < new_cap; count2++)
	new[count2] = '\0';

	free(old);
	return new;
}


/*This will read the entire file one more time, 
 * putting the encoded file as an array
 * in memory. */

char *getCode(int fd_in, char **codeArr){

        char *encoded;
        int cap = CHUNCK;
        int binCount = 0;
        uint8_t *currentChar;
        int ret = 1;
        encoded = malloc(cap);
        currentChar = malloc(BIG_STREAM);

        alloc_check(encoded);
        alloc_check(currentChar);

        /*Reading file and encoding it*/
        lseek(fd_in,0, SEEK_SET);
        ret = read(fd_in, currentChar, BIG_STREAM);
  	io_check(ret);	
        while (ret != 0){
                int iter;
		int count = 0;
		while (count < ret){
		*currentChar = *(currentChar + count);
                iter = 0;
		 /*Grow char array when needed*/
                if ( binCount + SAFE_MARGIN > cap){
                        cap = 2*cap;
                        encoded[binCount] = '\0';
                        encoded = myrealloc(encoded, cap);

                }

                /*Reading the code for each character in the file*/
                while (codeArr[(int) currentChar[0]][iter] != '\0'){
                        encoded[binCount] = codeArr[(int)currentChar[0]][iter];
                        binCount++;
                        iter++;
                }
		
		count++;
		}
                ret = read(fd_in, currentChar, BIG_STREAM);
		io_check(ret);

        }


        /*NULL terminating the code array*/
        encoded[binCount] = '\0';
        return encoded;

}




/* This will write the header to the ouput file*/
void writeHeader(int fd, uint32_t *freqArr){
	uint8_t *num_1;	
	int count;
        uint8_t *currChar;
        uint32_t *currFreq;
	uint8_t *buff = malloc(1);
	num_1 = malloc(1);	

	alloc_check(buff);
	alloc_check(num_1);

	*num_1 = 0;
	currChar = malloc(sizeof(uint8_t));
	currFreq =  malloc(sizeof(uint32_t));

	alloc_check(currChar);
	alloc_check(currFreq);
	
	if (currChar == NULL || currFreq == NULL)
                exit(1);
	for(count = 0; count < SIZE; count++){
		if (freqArr[count]){
			(*num_1)++;}
	}

	/*After all, it's num -1. Let's write it*/
	(*num_1)--;
	io_check(write(fd, num_1, 1));
	/*Writing frequencies and characters*/ 	
	for(count = 0; count < SIZE; count++){
                if (freqArr[count]){
         		*currChar = count;
			*currFreq = freqArr[count];            
			io_check( write(fd, currChar, 1));		
			/*Convert to big endianess(network)*/
			*currFreq = htonl(*currFreq);
			if ( write(fd, currFreq, 4) != 4){
				perror("writing freq.");
				exit(1);
			}
		 }
        }
	free(buff);
	free(currChar);
	free(currFreq);
}

/*This will write the body of the output*/
void writeBody(int fd_out, char *encoded){
        int count = 0;
        int byte_cap = 8, mask;
        uint8_t *out_byte = malloc(BIG_STREAM);
	int byteCount = 0;
	int iterator;
        alloc_check(out_byte);

	while (encoded[count] != '\0'){
		byteCount = 0;
		while(byteCount < BIG_STREAM){
		if (encoded[count] == '\0'){
			io_check(write(fd_out, out_byte, byteCount));
               		free(out_byte);
			return;}
		iterator = 0;
                mask = 0x80;
                *(out_byte + byteCount) = 0;
                while (iterator < byte_cap){
                        if ( encoded[count] == '1')
                        *(out_byte + byteCount) = *(out_byte + 
					     byteCount) + mask;
                        mask >>=1;
                        iterator++;
                        count++;
                }
		byteCount++;
		}
		io_check(write(fd_out, out_byte, byteCount));

        }
        free(out_byte);
}



int main(int argc, char *argv[]){

	int fd_in;
	int *emptyFlag, *oneFlag;
	uint32_t *freqArr;
	Node **histogram;
	LLNode *ll;
	Node *bst;
	char **codeArr;
	int count;
	int fd_out;
	char *encodedIn;

	fd_in = open(argv[1], O_RDONLY);
	emptyFlag = malloc(sizeof(int));
	oneFlag = malloc(sizeof(int));
	freqArr = calloc(SIZE,  sizeof(uint32_t));

	alloc_check(freqArr);
	alloc_check(oneFlag);
 	alloc_check(freqArr);

	histogram = build_histogram(fd_in, emptyFlag, freqArr, oneFlag);


	if (argc == 3){
		fd_out = open(argv[2], O_RDWR | O_CREAT | O_TRUNC,
                 (S_IWUSR | S_IXUSR| S_IRUSR | S_IROTH | S_IXOTH | S_IWOTH) );
                if (fd_out == -1){
                        perror("failure when opening output file");
                        exit(1);
                }
        }
        else{
                fd_out = STDOUT_FILENO;}

	/* Checking for empty file*/
	if (*emptyFlag == 1){
		return 0;
	}

	if (*oneFlag == 1){
		writeHeader(fd_out, freqArr);
		return 0;
	}
	ll = createLL(histogram);	
	bst = createBST(ll);

	codeArr = malloc( SIZE * sizeof(char *));

	alloc_check(codeArr);	

	/* Initializing Array*/
	for (count = 0; count < SIZE; count++){
		codeArr[count] = NULL;
	}	

	codeArr	= getEncoding(bst, codeArr, ""); 

	/* Let the encoding begin*/
	writeHeader(fd_out, freqArr);
	encodedIn = getCode(fd_in, codeArr);
	writeBody(fd_out, encodedIn);
		

	/* Deallocating stuff*/	
	close(fd_in);
	close(fd_out);
	
	free(emptyFlag);
	for (count = 0; count < SIZE; count++){
        	free(histogram[count]);
       		if (codeArr[count] != NULL)
			free(codeArr[count]);
	 }
	free(encodedIn);
	free(histogram);
	free(codeArr);
	free(bst);
	free(freqArr);
	return 0;
}
