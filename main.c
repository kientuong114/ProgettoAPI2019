#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#define REL_HASH_TABLE_SIZE 5000
#define ENTITY_HASH_TABLE_SIZE 20000 //Should be more than enough
#define MAX_HASH_VALUE UINT_MAX
#define SCOREBOARD_HASH_TABLE_SIZE 5000 //Should be same to REL_HASH_TABLE_SIZE

#define MAX_HASH_MASK 0xFFFFFFFFFFFFFFFF //2^64
#define REL_HASH_TABLE_MASK 0x0 
#define ENTITY_HASH_TABLE_MASK 0x0
#define SCOREBOARD_HASH_TABLE_MASK 0x0

//Set this constant to 0 to void verbose log messages
const int verbose = 1;

//Set this constant to 0 to skip over NULL cells when printing hash tables
const int suppress_NULL = 1;

struct ent_rel_list;
struct ent;
struct rel;
struct rel_type;
struct gen;

typedef struct ent{
	char* name;
	struct ent* next;
	struct ent* prec;
	struct ent_rel_list* rel_list;
	//int ID;
} entity;

typedef struct rel{
	entity* from;
	entity* to;
	struct rel* next;
	struct rel* prec;
	//int ID;
} relation;

typedef struct rel_type{
	//Can do with only single linked to avoid space waste?
	char* name;
	struct rel_type* prec;
	struct rel_type* next;
	relation** hashtable;
	//int ID;
} relation_type;

typedef struct ent_rel_list{ //List of all the relations of which the entity is the receiver
	relation* relation;
	struct ent_rel_list* next;	
	struct ent_rel_list* prec;
} entity_relation_list;

typedef struct gen{ //Generic doubly linked list node, used to avoid warnings in the generic list_print() function
	struct gen* prec;
	struct gen* next;
} generic;

typedef struct rb_node{
	//Used for scoreboard purposes: order relation is greatest score first and alphanumerical in cases for which two nodes have the same score.
	struct rb_node* p;
	struct rb_node* left;
	struct rb_node* right;
	int score;	
	//char* name;
	entity* ent;
	char colour;	
} rb_node;

typedef struct rb_tree{
	rb_node* root;
	int size;
} rb_tree;

typedef struct scoreboard{
	rb_tree ent_tree;
	rb_node** hashtable;
} scoreboard_t;

typedef enum data_type{
	ENTITY,
	REL,
	REL_TYPE,
	REL_COUNT,
	ENT_REL_LIST
} data_t;

// Function Prototypes

void ent_list_head_insert(entity*, entity**);
void rel_list_head_insert(relation*, relation**);
void rel_type_list_head_insert(relation_type*, relation_type**);
entity* find_entity_in_list(entity*, char*);
relation* find_relation_in_list(relation*, char*, char*);
relation_type* find_relation_type_in_list(relation_type*, char*);
void list_print(void*, data_t);
void entity_hash_table_insert(entity**, entity*, unsigned int);
void relation_hash_table_insert(relation**, relation*, unsigned int);
entity* find_entity(entity**, char*);
unsigned int hash(unsigned int);
unsigned int string_compactor(int, ...);


//TODO: Check for failed mallocs on all functions
//TODO: Rehashing when load factor is greater than 0.75

void ent_list_head_insert(entity* new_entity, entity** list_head){
	//Inserts a relation as head of list_head
	if(verbose){
		printf("Inserting entity %s at head of list pointed by %p\n", new_entity->name, (void*)list_head);
		printf("Old head of list was %s\n", *list_head == NULL ? NULL : (*list_head)->name);
	}
	new_entity->next = *list_head;
	if(*list_head!=NULL){
		(*list_head)->prec = new_entity;
	}
	new_entity->prec = NULL;
	*list_head = new_entity;
}

void rel_list_head_insert(relation* new_relation, relation** list_head){
	//Inserts a relation as head of list_head
	if(verbose){
		printf("Inserting relation from %s to %s at head of list pointed by %p\n", new_relation->from->name , new_relation->to->name, (void*)list_head);
		if(*list_head == NULL){
			printf("Old head of list was NULL\n");
		} else {
			printf("Old head of list was from %s to %s\n", (*list_head)->next->from->name, (*list_head)->next->to->name);
		}
	}
	new_relation->next = *list_head;
	if(*list_head!=NULL){
		(*list_head)->prec = new_relation;
	}
	new_relation->prec = NULL;
	*list_head = new_relation;
}

void rel_type_list_head_insert(relation_type* new_rel_type, relation_type** list_head){
	//Inserts a relation type as head of list_head
	if(verbose){
		printf("Inserting relation type  %s at head of list pointed by %p\n", new_rel_type->name , (void*)list_head);
		printf("Old head of list was %s", *list_head == NULL ? NULL : (*list_head)->name);
	}
	new_rel_type->next = *list_head;
	if(*list_head!=NULL){
		(*list_head)->prec = new_rel_type;
	}
	new_rel_type->prec = NULL;
	*list_head = new_rel_type;
}

entity* find_entity_in_list(entity* list_head, char* name){
	while(list_head!=NULL){
		if(strcmp(list_head->name, name)==0){
			if(verbose){
				printf("Found entity in list\n");
			}
			return list_head;
		}
		list_head = list_head->next;
	}
	return NULL;
}

relation* find_relation_in_list(relation* list_head, char* from, char* to){
	while(list_head!=NULL){
		if(strcmp(list_head->from->name, from)==0 && strcmp(list_head->to->name, to) == 0 ){
			if(verbose){
				printf("Found relation in list\n");
			}
			return list_head;
		}
		list_head = list_head->next;
	}
	return NULL;
}

relation_type* find_relation_type_in_list(relation_type* list_head, char* name){
	while(list_head!=NULL){
		if(strcmp(list_head->name, name)==0){
			if(verbose){
				printf("Found relation type in list\n");
			}
			return list_head;
		}
		list_head = list_head->next;
	}
	return NULL;
}
void list_print(void* list_head, data_t node_type){
	//Prints all contents of a list
	list_head = (generic*)list_head;
	while(list_head != NULL){
		switch(node_type){
			case ENTITY:
				printf("Entity Name: %s\n", ((entity*)list_head)->name);
				break;
			case REL:
				printf("Relation from %s to %s\n", ((relation*)list_head)->from->name, ((relation*)list_head)->to->name);
				break;
			case REL_TYPE:
				printf("Relation Type: %s\n", ((relation_type*)list_head)->name);
				break;
			case ENT_REL_LIST:
				printf("Relation from %s to %s\n", ((entity_relation_list*)list_head)->relation->from->name, ((entity_relation_list*)list_head)->relation->to->name);
				break;
			default:
				printf("Node of Unknown Type\n");
		}
		list_head = ((generic*)list_head)->next;
	}	
}

void entity_hash_table_insert(entity** hashtable, entity* ent, unsigned int position){
	ent_list_head_insert(ent, &hashtable[position]); 
}

void relation_hash_table_insert(relation** hashtable, relation* rel, unsigned int position){
	rel_list_head_insert(rel, &hashtable[position]);
}

entity* find_entity(entity** hashtable, char* name){
	unsigned int position = hash(string_compactor(1, name));
	return find_entity_in_list(hashtable[position], name);
}

relation** get_new_rel_hash_table(){
	//Creates a new Relation Hash Table, initializes it to NULL and returns a pointer to said table
	relation** hash_table_ptr= calloc(REL_HASH_TABLE_SIZE, sizeof(*hash_table_ptr));
	//int i;
	/*  
	for(i=0; i<REL_HASH_TABLE_SIZE; i++){
		hash_table_ptr[i] = NULL;
	}
	*/
	return hash_table_ptr;
}

entity** get_new_entity_hash_table(){
	//Creates a new Entity Hash Table, initializes it to NULL and returns a pointer to said table.
	entity** hash_table_ptr = calloc(ENTITY_HASH_TABLE_SIZE, sizeof(*hash_table_ptr));
	//int i;
	/*
	for(i=0; i<ENTITY_HASH_TABLE_SIZE; i++){
		hash_table_ptr[i] = NULL;
	}
	*/
	return hash_table_ptr;
}

void print_hash_table(void** hashtable, size_t hash_table_size,  data_t table_type){
	int i;
	for(i=0; i<hash_table_size; i++){
		if(hashtable[i]==NULL){
			if(suppress_NULL){
				continue;
			}
			printf("Position: %d - Content: NULL\n", i);
		} else {
			printf("Position: %d - Non empty cell: printing list...\n", i);
			list_print(hashtable[i], table_type);
		}
	}	
}

rb_node** get_new_rb_node_hash_table(){
	rb_node** hash_table_ptr = calloc(SCOREBOARD_HASH_TABLE_SIZE, sizeof(*hash_table_ptr));
	return hash_table_ptr;
}

int rb_node_compare(rb_node* x, rb_node* y){ //Acts as a > 'greater than' sign
	if(x->score > y->score){
		return 1;
	} else if (x->score < y->score){
		return 0;
	} else {
		return strcmp(x->ent->name, y->ent->name)<0?1:0;
	}
}
/* 
rb_node* rb_tree_search(rb_node* x, int score, char* name){
	if(x == NULL || (score == x->score && strcmp(x->ent->name, name)==0)){
		return x;
	}	
	if(score < x->score){
		return rb_tree_search(x->left, score);
	} else {
		return rb_tree_search(x->right, score);
	}
}
*/

void rb_left_rotate(rb_tree* tree, rb_node* x){
	rb_node* y = x->right;
	x->right = y->left;
	if(y->left != NULL){
		y->left->p = x;
	}	
	y->p = x->p;
	if(x->p == NULL){
		tree->root = y;
	}else if(x==x->p->left){
		x->p->left = y;
	}else{
		x->p->right = y;
	}
	y->left = x;
	x->p = y;
}

void rb_right_rotate(rb_tree* tree, rb_node* x){ 
	rb_node* y = x->left;
	x->left = y->right;
	if(y->right != NULL){
		y->right->p = x;
	}	
	y->p = x->p;
	if(x->p == NULL){
		tree->root = y;
	}else if(x==x->p->right){
		x->p->right = y;
	}else{
		x->p->left = y; 
	} 
	y->right = x; 
	x->p = y; 
} 

rb_node* rb_tree_minimum(rb_node* z){
	while(z->left != NULL){
		z = z->left;
	}
	return z;
}

rb_node* rb_tree_maximum(rb_node* z){
	while(z->right != NULL){
		z = z->right;
	}
	return z;
}

rb_node* rb_tree_successor(rb_node* z){
	if(z->right != NULL){
		return rb_tree_minimum(z->right);
	}
	rb_node* y = z->p;
	while(y!=NULL && z == y->right){
		z = y;
		y = y->p;
	}
	return y;
}

void rb_insert_fixup(rb_tree* tree, rb_node** z){
	rb_node *x, *y;
	if(*z == tree->root){
		tree->root->colour = 'b';
	}else{
		x = (*z)->p;
		if(x->colour == 'r'){
			if(x == x->p->left){
				y = x->p->right;
				if(y!= NULL && y->colour == 'r'){
					x->colour = 'b';
					y->colour = 'b';
					x->p->colour = 'r';
					rb_insert_fixup(tree, &(x->p));
				} else {
					if (*z == x->right){
						*z = x;
						rb_left_rotate(tree, *z);
						x = (*z)->p;
					}
					x->colour = 'b';
					x->p->colour = 'r';
					rb_right_rotate(tree, (x->p));
				}
			} else {
				if(x == x->p->right){
					y = x->p->left;
					if(y->colour == 'r'){
						x->colour = 'b';
						y->colour = 'b';
						x->p->colour = 'r';
						rb_insert_fixup(tree, &(x->p));
					} else {
						if (*z == x->left){
							*z = x;
							rb_right_rotate(tree, *z);
							x = (*z)->p;
						}
						x->colour = 'b';
						x->p->colour = 'r';
						rb_left_rotate(tree, (x->p));
					}
				}
			}
		}
	}
}

void rb_delete_fixup(rb_tree* tree, rb_node** x, rb_node** parent,  char side){
	rb_node* w;
	if((*x)!=NULL && ((*x)->colour == 'r' || (*x)->p == NULL)){
		(*x)->colour = 'b';
	} else if(side == 'l'){
		w = (*parent)->right;
		if(w->colour == 'r'){
			w->colour = 'b';
			(*parent)->colour = 'r';
			rb_left_rotate(tree, *parent);
		}
		if((w->left == NULL || w->left->colour == 'b') && (w->right == NULL || w->right->colour == 'b')){
			w->colour = 'r';
			if((*parent)->p==NULL){
				rb_delete_fixup(tree, parent, NULL, 'l');
			} else if(*parent == (*parent)->p->left){
				rb_delete_fixup(tree, parent, &((*parent)->p), 'l');
			} else {
				rb_delete_fixup(tree, parent, &((*parent)->p), 'r');
			}
		} else {
			if(w->right == NULL || w->right->colour == 'b'){
				w->left->colour = 'b';
				w->colour = 'r';
				rb_right_rotate(tree, w);
				w = (*parent)->right;
			}
			w->colour = (*parent)->colour;
			(*parent)->colour = 'b';
			w->right->colour = 'b';
			rb_left_rotate(tree, *parent);
		}
	} else if(side == 'r') {
		w = (*parent)->left;
		if(w->colour == 'r'){
			w->colour = 'b';
			(*parent)->colour = 'r';
			rb_right_rotate(tree, *parent);
		}
		if((w->right == NULL || w->right->colour == 'b') && (w->left == NULL || w->left->colour == 'b')){
			w->colour = 'r';
			if((*parent)->p == NULL){
				rb_delete_fixup(tree, parent, NULL, 'l');
			} else if(*parent == (*parent)->p->left){
				rb_delete_fixup(tree, parent, &((*parent)->p), 'l');
			} else {
				rb_delete_fixup(tree, parent, &((*parent)->p), 'r');
			}

		} else {
			if(w->left->colour == 'b'){
				w->right->colour = 'b';
				w->colour = 'r';
				rb_left_rotate(tree, w);
				w = (*parent)->left;
			}
			w->colour = (*parent)->colour;
			(*parent)->colour = 'b';
			w->left->colour = 'b';
			rb_right_rotate(tree, *parent);
		}
	} else {
		printf("Invalid Side\n");
		return;
	}
}

void rb_delete(rb_tree* tree, rb_node** z){
	rb_node* y = NULL;
	rb_node* x = NULL;
	char side;
	if((*z)->left == NULL || (*z)->right == NULL){
		 y = *z;
	} else {
		y = rb_tree_successor(*z);		
	}
	if(y->left != NULL){
		x = y->left;
	} else {
		x = y->right;
	}
	if(x != NULL){
		x->p = y->p;
	}
	if(y->p == NULL){
		tree->root = x;
	} else if (y == y->p->left){
		side = 'l';
		y->p->left = x;
	} else {
		side = 'r';
		y->p->right = x;
	}
	if (y != *z){
		(*z)->score = y->score;
	}
	if (y->colour == 'b'){	
		rb_delete_fixup(tree, &x, &(y->p), side);
	}
	free(y);
	(tree->size)--;
}

void rb_insert(rb_tree* tree, rb_node** z){
	rb_node* y = NULL;
	rb_node* x = tree->root;
	while(x!=NULL){
		y = x;
		//if((*z)->score < x->score){
		if(!rb_node_compare(*z, x)){
			x = x->left;
		} else {
			x = x->right;
		}
	}	
	(*z)->p = y;
	if(y == NULL){
		tree->root = *z;
	//}else if((*z)->score < y->score){
	} else if(!rb_node_compare(*z, y)) {
		y->left = *z;
	} else {
		y->right = *z;
	}
	(*z)->left = NULL;
	(*z)->right = NULL;
	(*z)->colour = 'r';
	rb_insert_fixup(tree, z);
	(tree->size)++;
}

void rb_inorder_tree_walk(rb_node* node){
	if(node->left != NULL){
		rb_inorder_tree_walk(node->left);
	} else {
		printf("Hit a NULL node\n");
	}	
	printf("Printing node with score %d and colour %c\n", node->score, node->colour);
	if(node->right != NULL){
		rb_inorder_tree_walk(node->right);
	} else { 
		printf("Hit a NULL node\n");
	}
}

void rb_preorder_tree_walk(rb_node* node){
	printf("Printing node with score %d and colour %c\n", node->score, node->colour);
	if(node->left != NULL){
		rb_preorder_tree_walk(node->left);
	} else {
		printf("Hit a NULL node\n");
	}	
	if(node->right != NULL){
		rb_preorder_tree_walk(node->right);
	} else { 
		printf("Hit a NULL node\n");
	}
}

unsigned int string_compactor(int num, ...){
	//As a sort of hash, this function works upon multiple strings and yields a number which is a known function of the input strings.
	//This works well under the assumptions that the names for the entities and the rel_type, along with the from and to fields are uniformly distributed, which seem like a reasonable prior.
	va_list arglist;
	int i, j;
	char* currString;
	unsigned int currSum = 0;
	va_start(arglist, num);
	for(i=0; i<num; i++){
		currString = va_arg(arglist, char*);	
		for(j=0; j<strlen(currString); j++){
			if(j%7==0){
				//currSum = ((currSum<<4)^((int)currString[j]<<23))%MAX_HASH_VALUE;
				currSum = ((currSum<<4)^((int)currString[j]<<23)) & MAX_HASH_MASK;
			}else if(j%13==0){
				//currSum = ((currSum<<4)^((int)currString[j]<<17))%MAX_HASH_VALUE;
				currSum = ((currSum<<4)^((int)currString[j]<<17)) & MAX_HASH_MASK;
			}else{
				//currSum = ((currSum<<10)^(11*currString[j])<<8)%MAX_HASH_VALUE;
				currSum = ((currSum<<10)^(11*currString[j])<<8) & MAX_HASH_MASK;
			}
		}
	}
	return currSum;
}

unsigned int hash(unsigned int key) {
	//Not really a hash, more like a function to avoid any 
	return key%ENTITY_HASH_TABLE_SIZE;
}
/* 
void add_new_relation(entity* from, entity* to, relation_type* rel_type){
	newRel->ID = hash(from->ID + to->ID + rel_type->ID);
	rel_head_insert(&newRel, &(rel_type->hashtable[newRel->ID]));
}
*/

entity* add_new_entity(entity** hashtable, char* name){
	entity* newEnt = malloc(sizeof(entity));
	newEnt->name = malloc(strlen(name)+1);
	strcpy(newEnt->name, name);
	unsigned int position = hash(string_compactor(1,name));
	entity_hash_table_insert(hashtable, newEnt, position);	
	return newEnt;
}

relation* add_new_relation(entity* from, entity* to, relation_type* rel_list){
	relation** rel_hash_table = rel_list->hashtable;
	relation* newRel = malloc(sizeof(relation));
	newRel->from = from;
	newRel->to = to;
	unsigned int position = hash(string_compactor(3, rel_list->name, from->name, to->name));
	relation_hash_table_insert(rel_hash_table, newRel, position); 
	return newRel;
}

void initialize_structure(entity*** hash_table, relation_type** relation_type_list){
	//Initializes every data structure necessary for the program
	*hash_table = get_new_entity_hash_table();
	*relation_type_list = NULL;
}

void addent(){
}

void delent(){

}

void addrel(){

}

void delrel(){
	
}

void report(){

}

void testSuite1(entity** global_entity_hash_table){
	add_new_entity(global_entity_hash_table, "Giorgio");
	add_new_entity(global_entity_hash_table, "Gianni");
	add_new_entity(global_entity_hash_table, "Giorgdnskjaio");
	add_new_entity(global_entity_hash_table, "Giorgio2");
	add_new_entity(global_entity_hash_table, "Giorgio3");
	add_new_entity(global_entity_hash_table, "Giorgio43");
	add_new_entity(global_entity_hash_table, "Giorgio5kj");
	add_new_entity(global_entity_hash_table, "Giorgio232");
	add_new_entity(global_entity_hash_table, "Giorgio9");
	add_new_entity(global_entity_hash_table, "GiorgioGianni");
	add_new_entity(global_entity_hash_table, "GiorgioNove");
	add_new_entity(global_entity_hash_table, "GiorgioLol");
	print_hash_table((void**)global_entity_hash_table, ENTITY_HASH_TABLE_SIZE, ENTITY);
	printf("Finding Giorgio43: %s\n", find_entity(global_entity_hash_table, "Giorgio43")->name);
	printf("Finding inexistant entity: %p\n", (void*)find_entity(global_entity_hash_table, "Kien"));
}

int main(){
	entity** global_entity_hash_table;
	relation_type* global_relation_type_list;
	initialize_structure(&global_entity_hash_table, &global_relation_type_list);
	testSuite1(global_entity_hash_table);
}

