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
Node **headerToHistogram(int fd_in, long int *total, int *oneChar){
        Node **histogram = malloc(SIZE*sizeof(Node *));
        int count;
	uint8_t *num_1 = malloc(1), *currentChar = malloc(1);
	uint32_t *currentFreq = malloc(4);
	
	if (num_1 == NULL || currentChar == NULL ||
		currentFreq == NULL)
	exit(1);
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
        read(fd_in, num_1, 1);

	if (*num_1 == 0){
		*oneChar = 1;	
	}	
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
		*total = *total + freqq;		
	}
	free(num_1); 
	free(currentChar);
	free(currentFreq);
	return histogram;
}

/*Compares two nodes, and sorts histogram in order
 *  * to create initial linked list. Histogram will be 
 *   * sorted in the following order:
 *    * 1 - Freq = 0, highest order (put in end of array)
 *     * 2 - Lower frequencies come first
 *      * 3 - Lower chars come first*/
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
	
	int count, a= 0;
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
		LLNode *node_ll = malloc(sizeof(LLNode *) + 
					sizeof(Node *));		
		if(node_ll == NULL)
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

void writeOutput(int fd_in, int fd_out, Node *bst, long int *sumChar){
	
	uint8_t *current = malloc(1);
	int mask;
	Node *node = bst; /*Start at root*/
	uint8_t *buff = malloc(1);
	int charWritten = 0;
	while (  read(fd_in, current, 1) != 0){
		for ( mask = 0x80; mask; mask>>= 1){
	
			/*If at a leaf ...*/
			if (node -> right == NULL &&  node -> left == NULL){ 
				charWritten++;
				*buff = (uint8_t) node -> Char;
				if (write(fd_out, buff, 1) != 1){
					perror("writing to out");
					exit(1);

				}

				/*Restart at the root*/
				node = bst;
			}
			if (charWritten == *sumChar)
				exit(0);		
			/*Go right*/
			if (mask & *current){
				node = node -> right;

			/*Go left*/
			}else{
				node = node -> left;	
			}	
		}
	}
	free(buff);
}

void writeOneChar(Node **histogram, int fd_out){

	uint8_t *oneChar;
	int freq, count;
	
	oneChar = malloc(sizeof(int *));
	
	for (count = 0; count < SIZE; count++){
	
		if (histogram[count] -> freq != 0){
		*oneChar = count;
		freq = histogram[count] -> freq;
		break;
		}	
	}

	for (count = 0; count < freq; count++){
		write(fd_out, oneChar, 1);
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
	long int *sumChar = malloc(sizeof(long int));
	if (oneChar == NULL || sumChar == NULL)
	exit(1);
	*sumChar = 0;
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
	/*Otherwhise, let's reset out pointer*/
		lseek(fd_in, 0, SEEK_SET);	
	}
	*oneChar = 0;
	histogram = headerToHistogram(fd_in, sumChar, oneChar);
	if (*oneChar == 1){
		writeOneChar(histogram, fd_out);
		free(histogram);	
		exit(0);
	}
	ll = createLL(histogram);
        bst = createBST(ll);
	writeOutput(fd_in, fd_out, bst, sumChar); 
		
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
