#include "getword.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAXWORD	100

/* most of with some of the keyword types, not all and an int to store count */
const char *datatypes[] = 
{
	"_BitInt", /*ANOTHER TRICKY ONE IT'S FORMAT LIKE _BitInt(32) SO WE NEED TO WORRY ABOUT () 
		       *MIGHT SKIP FOR SIMPLICITY */

	"_Bool",
	"_Complex",
	"_Decimal128",
	"_Decimal32",
	"_Decimal64",
	"bool",
	"char",
	"double",
	"float",
	"int",
	"int16_t",
	"int32_t",
	"int64_t",
	"int8_t",
	"long",
	"short",
	"struct", //WARNUNG, WHAT ARE WE GONNA DO ABOUT THESE?? 
	"uint16_t",
	"uint32_t",
	"uint64_t",
	"uint8_t",
	"void", //ALSO TRICKY WHAT ABOUT FUNCTION RETURNS I JUST THROUGHT OF THIS!!
};

#define NKEYS	sizeof datatypes / sizeof(char*)

int binsearch(char *, const char **, int);
static inline int exists(char *, const char **, int);
struct gnode *addgnode(char *, struct gnode *, int);
struct enode *addenode(char *, struct enode *);
void printtree(struct gnode *);

/* node for tree of groups (i.e. x first letter of words) */
struct gnode
{
	char *var_beg;
	struct enode *root;
	struct gnode *left;
	struct gnode *right;
};

/* struct for ends of variables in same group */
struct enode
{
	char *var_end;
	struct enode *left;
	struct enode *right;
};

int main(int argc, char **argv)
{
	if(argc > 2)
		printf("Usage: ./6-2 [groupsize]\n");
	int n, datatype, grpsz;
	char word[MAXWORD];
	struct attr wattr;
	datatype = 0;
	if(argc == 2)
		grpsz = atoi(argv[1]);
	else
		grpsz = 6;
	struct gnode *root = NULL;

	while((wattr = getword(word, MAXWORD)).c != EOF)
		if(datatype)
		{
			if((isalpha(word[0]) || word[0] == '_') && !wattr.str && !wattr.comm)
			{
				root = addgnode(word, root, grpsz);
				if(root == NULL) /* error indicated by the system */
					return EXIT_FAILURE; 
				datatype = 0;
				continue;
			}
		}
		else if((isalpha(word[0]) || word[0] == '_' || word[0] == '#') && !wattr.str && !wattr.comm)
			if(exists(word, datatypes, NKEYS))
				datatype = 1;
	printf("\nKeywords Found:\n");
	printtree(root);

	return EXIT_SUCCESS;
}

/* binsearch: find word in str[0]...str[n-1] */
int binsearch(char *word, const char *str[], int n)
{
	int cond;
	int low, high, mid;

	low = 0;
	high = n - 1;
	while(low <= high)
	{
		mid = (low + high) / 2;
		if((cond = strcmp(word, str[mid])) < 0)
			high = mid - 1;
		else if(cond > 0)
			low = mid + 1;
		else
			return mid;
	}

	return -1;
}

/* exists: wrapper function for binsearch that returns true if found instead of location */
static inline int exists(char *word, const char *wlist[], int len)
{
	return binsearch(word, wlist, len) == -1 ? 0 : 1;
}

/* addgnode: add (if not already present) the group part of variable */
struct gnode *addgnode(char *word, struct gnode *p, int grpsz)
{
	int comp;

	if(p == NULL)
	{
		p = malloc(sizeof(struct gnode));
		p->var_beg = strndup(word, grpsz); /* plus 1 because of '\0' */
		if(p->var_beg == NULL)
		{
			printf("Error: out of memory");
			return NULL;
		}
		p->var_beg[grpsz] = '\0';
		p->left = NULL;
		p->right = NULL;

		/* no need to add ending part to tree word is size or less than grpsz */
		if(strlen(word) <= grpsz)
			p->root = NULL;
		/* add ending part of word to ending part tree */
		else
			p->root = addenode(word+grpsz, p->root);
	}
	else if((comp = strncmp(word, p->var_beg, grpsz)) == 0)
	{
		if(strlen(word) > grpsz)
			p->root = addenode(word+grpsz, p->root);
	}
	else if(comp < 0)
		p->left = addgnode(word, p->left, grpsz);
	else
		p->right = addgnode(word, p->right, grpsz);

	return p;
}

/* enode: add ending part (if not already present) or variable name */
struct enode *addenode(char *end, struct enode *p)
{
	int comp;

	if(p == NULL)
	{
		p = malloc(sizeof(struct enode));
		p->var_end = strdup(end); /* plus 1 because of '\0' */
		if(p->var_end == NULL)
		{
			printf("Error: out of memory");
			return NULL;
		}
		p->left = NULL;
		p->right = NULL;

	}
	else if((comp = strcmp(end, p->var_end)) < 0)
		p->left = addenode(end, p->left);
	else if(comp > 0)
		p->right = addenode(end, p->right);

	return p;

}

/* printtree: */
void printtree(struct gnode *p)
{
	void printetree(struct enode*, char*); /* internal function declaration */
	if(p != NULL)
	{
		printtree(p->left);
		/* this first x characters maybe there was nothing after it just the group 
		 * i.e. group is 5 characters. Variable name is just n. Nothing to overflow 
		 * into end variable group */
		if(p->root == NULL)
		{
			printf(p->var_beg);
			printf("\n");
		}
		else
			printetree(p->root, p->var_beg);
			//printf("\n");
		printtree(p->right);
	}
}

/* printetree: internally used function by printtree to get ending parts from groups */
void printetree(struct enode *p, char *var_beg)
{
	if(p != NULL)
	{
		printetree(p->left, var_beg);
		printf("%s%s\n", var_beg, p->var_end);
		printetree(p->right, var_beg);
	}
}
