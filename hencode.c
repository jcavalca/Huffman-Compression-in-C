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
# define CODE_SIZE 256
# define CHUNCK 500
# define SAFE_MARGIN 100
# define BIG_STREAM 4096

/*Builds initial histogram*/
Node **build_histogram(int fd, int *emptyFlag, uint32_t *freqArr, int *oF){
	Node **histogram;
	int currentChar;
	int count, onec;
	uint8_t *in = malloc(sizeof(char));
	histogram = malloc(SIZE*sizeof(Node *));
	if (histogram == NULL || in == NULL){
		perror("malloc");
		exit(1);
	}

	/* Initializing histogram*/
	for (count = 0; count < SIZE; count ++){
		Node *huffNode = malloc(2*sizeof(int) + 2*sizeof(Node *));
		if (huffNode == NULL)
		exit(1);
		huffNode -> Char = count;
		huffNode -> freq = 0;
		huffNode -> left = NULL;
		huffNode -> right = NULL;
		histogram[count] = huffNode;
	}
	
	/* Reading and filling up histogram*/
	currentChar = read(fd, in, 1);
	
	while (currentChar != 0){
		(histogram[(int) in[0]] -> freq)++;
		currentChar = read(fd, in, 1);
		count++;
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

/*Compares two nodes, and sorts histogram in order
 * to create initial linked list. Histogram will be 
 * sorted in the following order:
 * 1 - Freq = 0, highest order (put in end of array)
 * 2 - Lower frequencies come first
 * 3 - Lower chars come first*/
int compareNode(const void *p1, const void *p2){
        
        Node *node1 = * (Node * const *) p1;
        Node *node2 = * (Node * const *) p2 ;
        
        int char1 = node1 -> Char;
        int char2 = node2 -> Char;
                
        int freq1 = node1 -> freq;
        int freq2 = node2 -> freq;

	/*If one of the two nodes have frequency 0*/
	if (freq1 == 0 || freq2 == 0){
		if (freq1 == 0){
			return 1;
		}
		else if (freq2 == 0){
			return -1;
		}else{
			return 0;
		}
	}
	
        /* Lower frequency will be first in array*/
	
	else if (freq1 != freq2){
		return freq1 - freq2;
	}               

        /* Frequency ties, lower chars come first*/
	else{
		return char1 - char2;
	}
		
}

/* Creates a LL with the histogram data*/
LLNode *createLL(Node **histogram){
	
	int count, a = 0;
	int numbElements = 0;
	LLNode *nodesArr[SIZE];
	qsort(histogram, SIZE, sizeof(Node *), compareNode);
	/*Initializing linked list with their data*/
	for (count = 0; count < SIZE; count++){

		/*Non used chars are irrelevant*/
		if (histogram[count] -> freq != 0){
		LLNode *node_ll = malloc(sizeof(LLNode *) + 
					sizeof(Node *));		
		if (node_ll == NULL)
                exit(1);	
		node_ll -> data = histogram[count];
		nodesArr[a] = node_ll;
		numbElements++;
		a++;
		}
	}

	/*Setting up the links*/

	/* Only one char*/
	if (numbElements == 1){
		nodesArr[0] -> next = NULL;
		return nodesArr[0];	
	}
		
	for (count = 0; count < numbElements - 1; count++){
		nodesArr[count] -> next =  nodesArr[count + 1];
	}
	nodesArr[count] -> next = NULL;
	
	return nodesArr[0];
}

/* Removes first 2 nodes and adds the combined one*/
LLNode *delete2add1(LLNode *linkedList){
	Node *left = linkedList -> data;
	Node *right = linkedList -> next -> data;
	LLNode *current, *new;
	LLNode *first = linkedList;
	LLNode *sec = linkedList -> next;
	Node *combined;

	linkedList = linkedList -> next -> next;
	current = linkedList;
	
	free(first);
	free(sec);
	
	combined = malloc(2*sizeof(int) + 2*sizeof(Node *));
	if (combined == NULL)
                exit(1);
	combined -> freq = (left -> freq) + (right -> freq);
	combined -> Char = -1; /* Combined node flag */
	combined -> left = left;
	combined -> right = right;


	new = malloc(sizeof(Node *) + sizeof(LLNode *));
 	if (new == NULL)
                exit(1);
	new -> data = combined;
        new -> next = NULL;

	/* Combined node is the only one left */

	if (linkedList == NULL){
		linkedList = new;
		return linkedList;
	}	

	/* Checking if we need to replace head */ 
	if (linkedList -> data -> freq >= new -> data -> freq){

		new -> next = linkedList;
		linkedList = new;
		return linkedList;
	}

	/* Adding in between two nodes */	
	while (current -> next != NULL){
		if (current -> next -> data -> freq >= new -> data -> freq){
			LLNode *oldNext = current -> next;
			current -> next = new;
			new -> next = oldNext;
			return linkedList;
		}

		current = current -> next;
	}


	/* Addding in the tail*/
	current -> next = new;
	return linkedList;
}
/* Creates the BST containing the encoding*/
Node *createBST(LLNode *linkedList){
	Node *ret;
	while(linkedList -> next != NULL)
		linkedList = delete2add1(linkedList);
	ret = linkedList -> data;
	free(linkedList);
	return ret;
}

/* Writing a simple implementation of strcat*/
char *mystrcat(char *word1, char *word2){
	int new_len = strlen(word1) + strlen(word2) + 1;
	int count;
	int count2 = 0;
	int len1 = strlen(word1);
	char *new_word = malloc( new_len * sizeof(char));
	if (new_word == NULL)
                exit(1);
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
	if(new==NULL)
	exit(1);
	
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
 * outputting compression as it goes.*/

char *getCode(int fd_in, char **codeArr){

	
	char *encoded;
	int cap = CHUNCK;
	int binCount = 0;
	uint8_t *currentChar;
	int ret = 1;

	encoded = malloc(cap);
	currentChar = malloc(1);
	if (encoded == NULL || currentChar == NULL)
                exit(1);

	/*Reading file and encoding it*/
	lseek(fd_in,0, SEEK_SET);
	ret = read(fd_in, currentChar, 1);
	
	while (ret != 0){
		 int iter = 0;
		
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
	
		ret = read(fd_in, currentChar, 1);

		
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
	if(buff == NULL)
	exit(1);
	*num_1 = 0;
	currChar = malloc(sizeof(uint8_t));
	currFreq =  malloc(sizeof(uint32_t));
	if (currChar == NULL || currFreq == NULL)
                exit(1);
	for(count = 0; count < SIZE; count++){
		if (freqArr[count]){
			(*num_1)++;}
	}

	/*After all, it's num -1. Let's write it*/
	(*num_1)--;
	write(fd, num_1, 1);

	/*Writing frequencies and characters*/ 	
	for(count = 0; count < SIZE; count++){
                if (freqArr[count]){
         		*currChar = count;
			*currFreq = freqArr[count];            
			
			write(fd, currChar, 1);
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
	uint8_t *out_byte = malloc(1);
	if (out_byte == NULL)
	exit(1);
	
	while (encoded[count] != '\0'){
		int iterator = 0;
		mask = 0x80;	
		*out_byte = 0;
		while (iterator < byte_cap){
			if ( encoded[count] == '1')
				*out_byte = *out_byte + mask;			
			mask >>=1;
			iterator++;
			count++;
		}
		write(fd_out, out_byte, 1);
	}
	free(out_byte);
}
/*
void writeBody2(char **codeArr, Node **histogram, int fd_in, int fd_out){
	
	long int totalChar = 0;
	int count = 0;
	int ret = 1;
	char *buff = calloc(BIG_STREAM, 1);

	if(buff == NULL){exit(1);}	

	for (count = 0; count < SIZE; count ++){
	if (histogram[count] -> freq != 0){
	int prod = (histogram[count] -> freq) * strlen(codeArr);
	totalChar = totalChar + prod;
	}
	}
	
	read(fd_in, buff, BIG_STREAM);

	}




}
*/




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
	if(emptyFlag == NULL || freqArr == NULL || oneFlag == NULL)
	exit(1);
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

	/* Initializing Array*/
	for (count = 0; count < SIZE; count++){
		codeArr[count] = NULL;
	}	

	codeArr	= getEncoding(bst, codeArr, ""); 

	/* Let the encoding begin*/
	writeHeader(fd_out, freqArr);
	encodedIn = getCode(fd_in, codeArr);
	writeBody(fd_out, encodedIn);
		
	/* DEBUG
	printf("%s \n", encodedIn);
	for (count = 0; count < SIZE; count++){
 		if (codeArr[count] != NULL){
		printf("\n0x%02x: %s\n", count, codeArr[count]);
				
		}      

	 } */
	

	


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
