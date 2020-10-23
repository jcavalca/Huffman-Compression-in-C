# include <stdlib.h>
# include <stdio.h>
# include <string.h>


# define SIZE 256
# define CODE_SIZE 256


/*This functions checks memory allocation*/
void alloc_check(void *pp){
        if (pp == NULL){
                perror("memory allocation");
                exit(1);
        }
}

/*This function checks read/write sys calls*/
void io_check(int ret){
	if (ret < 0){
		perror("I/O");
		exit(1);
	}
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
                alloc_check(node_ll);
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
        LLNode *first = linkedList;
        LLNode *second = linkedList -> next;
        linkedList = linkedList -> next -> next;
        current = linkedList;

        free(first);
        free(second);


        combined = malloc(2*sizeof(int) + 2*sizeof(Node *));
        alloc_check(combined);
        combined -> freq = (left -> freq) + (right -> freq);
        combined -> Char = -1; /* Combined node flag */
        combined -> left = left;
        combined -> right = right;


        new = malloc(sizeof(Node *) + sizeof(LLNode *));
        alloc_check(new);
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


