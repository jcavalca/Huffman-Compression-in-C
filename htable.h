/*
 * Defines structures and prototypes.
 * CPE 357, Lab3
 * */

typedef struct Node
{
	int Char;
	int freq;
	struct Node *left;
	struct Node *right;
} Node;

typedef struct LLNode
{
	struct Node *data;
        struct LLNode *next;
} LLNode;

Node **build_histogram(int file_descriptor, int *emptyFlag);
int compareNode(const void *p1, const void *p2);
LLNode *createLL(Node **histogram);
LLNode *delete2add1(LLNode *linkedList);
Node *createBST(LLNode *linkedList);
