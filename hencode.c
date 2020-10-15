# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
#include <unistd.h>

# include "htable.h"

# define SIZE 256
# define CODE_SIZE 256

# define CHUNCK 500

/*Builds initial histogram*/
Node **build_histogram(int fd, int *emptyFlag){
	Node **histogram = malloc(SIZE*sizeof(Node *));
	int currentChar;
	int count;
	char *in = malloc(sizeof(char));

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
	currentChar = read(fd, in, 1);
	
	/* Empty file*/
	if (currentChar == 0){
		*emptyFlag = 1;}
	else{*emptyFlag = 0;}

	while (currentChar != 0){
		(histogram[(int) in[0]] -> freq)++;
		currentChar = read(fd, in, 1);
	}		

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
	
	int count;
	int numbElements = 0;
	LLNode *nodesArr[SIZE];
	qsort(histogram, SIZE, sizeof(Node *), compareNode);

	/*Initializing linked list with their data*/
	for (count = 0; count < SIZE; count++){

		/*Non used chars are irrelevant*/
		if (histogram[count] -> freq == 0){
			break;
		}
		else{
LLNode *node_ll = malloc(sizeof(LLNode *) + sizeof(Node *));		
		node_ll -> data = histogram[count];
		nodesArr[count] = node_ll;
		numbElements++;
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
	Node *combined;

	linkedList = linkedList -> next -> next;
	current = linkedList;

	combined = malloc(2*sizeof(int) + 2*sizeof(Node *));
	combined -> freq = (left -> freq) + (right -> freq);
	combined -> Char = -1; /* Combined node flag */
	combined -> left = left;
	combined -> right = right;


	new = malloc(sizeof(Node *) + sizeof(LLNode *));
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

	while(linkedList -> next != NULL)
		linkedList = delete2add1(linkedList);
	return linkedList -> data;
}

/* Writing a simple implementation of strcat*/
char *mystrcat(char *word1, char *word2){
	int new_len = strlen(word1) + strlen(word2) + 1;
	int count;
	int count2 = 0;
	int len1 = strlen(word1);
	char *new_word = malloc( new_len * sizeof(char));
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
		int len = strlen(code);	
		char *str = malloc( (len + 1)*sizeof(char));
		str = code;
		codeArr[node -> Char] = str;
	}
	if (node -> left != NULL){
		new_code = mystrcat(code, "0");
		getEncoding(node -> left, codeArr, new_code);
	}
	if (node -> right != NULL){
		new_code = mystrcat(code, "1");		 
	      	getEncoding(node -> right, codeArr, new_code);
        }
	return codeArr;
}


/*This will read the entire file one more time, 
 * translating everything into 0s and 1s according
 * to the Huffman Tree into a char array.*/
char *getCode(int fd, char **codeArr){
	
	char *encoded = malloc( CHUNCK * sizeof(char));
	int cap = CHUNCK;
	int binCount = 0;
	int *currentChar = malloc(sizeof(int));
	int ret = 1;
	/*Setting fd to begg of file*/
	
	lseek(fd, 0, SEEK_SET);

	/*Reading file and encoding it*/

	ret = read(fd, currentChar, 1);
	
	while (ret != 0){
		 int iter = 0;
		
		 /*Grow char array when needed*/
		if ( binCount + 100 > cap){
			cap = 2*cap;
			encoded = realloc(encoded, cap);
		}
	
		/*Reading the code for each character in the file*/
		while (codeArr[*currentChar][iter] != '\0'){
			encoded[binCount] = codeArr[*currentChar][iter];
			binCount++;
			iter++;
		}
		ret = read(fd, currentChar, 1);
	}
	

	/*NULL terminating the code array*/
	encoded[binCount] = '\0';
	return encoded;
}



int main(int argc, char *argv[]){

	int fd = open(argv[1], O_RDONLY);
	int *emptyFlag = malloc(sizeof(int));
	Node **histogram = build_histogram(fd, emptyFlag);	
	LLNode *ll;
	Node *bst;
	char **codeArr;
	int count;
	char *encodedFile;

	/* Checking for empty file*/
	if (*emptyFlag == 1){
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
	encodedFile = getCode(fd, codeArr);
	
				
	 for (count = 0; count < SIZE; count++){
 		if (codeArr[count] != NULL){
			printf("0x%02x: %s\n", count, codeArr[count]);
			
		}      

	 } 
	
	free(emptyFlag);
	for (count = 0; count < SIZE; count++){
        	free(histogram[count]);
       		if (codeArr[count] != NULL && strlen(codeArr[count]) > 1)
			free(codeArr[count]);
	 }
	free(histogram);
	free(codeArr);
	free(bst);
	free(ll);
	return 0;
}
