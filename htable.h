/*der(int fd, char **codeArr)
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


/* Huffman Tree Implementation*/
Node **build_histogram(int file_descriptor, int *emptyFlag, uint32_t *arr);
int compareNode(const void *p1, const void *p2);
LLNode *createLL(Node **histogram);
LLNode *delete2add1(LLNode *linkedList);
Node *createBST(LLNode *linkedList);
char *mystrcat(char *word1, char *word2);
char **getEncoding(Node *node, char **codeArr, char *code);



/* Encoding Output File Implementation */

char *getCode(int fd, char **codeArr);
void writeHeader(int fd, uint32_t *freqArr);
