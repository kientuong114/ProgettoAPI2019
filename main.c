/*
 * ======================================================================================================================================
 *
 *       Filename:  progettoAPI2019.c 
 *
 *    Description:  My submission for the final project of Algorithms and Data Structures (Algoritmi e Strutture Dati) for the course
 *    		    of Algorithms and Foundations of Computer Science (Algoritmi e Principi dell'Informatica), academic year 2018-2019.  
 *
 *        Version:  1.0
 *        Created:  28/06/2019 12:06:11
 *       Compiler:  gcc
 *
 *         Author:  Kien Tuong Truong (887907)  
 *
 * =====================================================================================================================================
 */

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
struct scoreboard;

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
	rb_node** hash_table;
} scoreboard_t;

typedef struct rel_type{
	//Can do with only single linked to avoid space waste?
	char* name;
	struct rel_type* prec;
	struct rel_type* next;
	relation** hash_table;
        scoreboard_t scoreboard;
	//int ID;
} relation_type;

typedef struct ent_rel_list{ //List of all the relations of which the entity is the receiver
	relation_type* rel_type;
	relation* rel;
	struct ent_rel_list* next;	
	struct ent_rel_list* prec;
} entity_relation_list;

typedef struct gen{ //Generic doubly linked list node, used to avoid warnings in the generic list_print() function
	struct gen* prec;
	struct gen* next;
} generic;



typedef enum data_type{
	ENTITY,
	REL,
	REL_TYPE,
	REL_COUNT,
	ENT_REL_LIST,
	SCOREBOARD
} data_t;

// Function Prototypes

void ent_list_head_insert(entity*, entity**);
void rel_list_head_insert(relation*, relation**);
void rel_type_list_head_insert(relation_type*, relation_type**);
entity* find_entity_in_list(entity*, char*);
relation* find_relation_in_list(relation*, char*, char*);
relation_type* find_relation_type_in_list(relation_type*, char*);
void entity_hash_table_insert(entity**, entity*, unsigned int);
void relation_hash_table_insert(relation**, relation*, unsigned int);
entity* find_entity(entity**, char*);
unsigned int hash(unsigned int, data_t);
unsigned int string_compactor(int, ...);
void initialize_new_rel_type(relation_type*);

//TODO: Check for failed mallocs on all functions
//TODO: Rehashing when load factor is greater than 0.75

void ent_list_head_insert(entity* new_entity, entity** list_head){
	//Inserts a relation as head of list_head
	if(verbose){
		printf("Inserting entity %s at head of list pointed by %p ", new_entity->name, (void*)list_head);
		printf("(Old head of list was %s)\n", *list_head == NULL ? "NULL" : (*list_head)->name);
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
		printf("Inserting relation from %s to %s at head of list pointed by %p ", new_relation->from->name , new_relation->to->name, (void*)list_head);
		if(*list_head == NULL){
			printf("(Old head of list was NULL)\n");
		} else {
			printf("(Old head of list was from %s to %s)\n", (*list_head)->next->from->name, (*list_head)->next->to->name);
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
		printf("Inserting relation type %s at head of list pointed by %p ", new_rel_type->name , (void*)list_head);
		printf("(Old head of list was %s)\n", *list_head == NULL ? "NULL" : (*list_head)->name);
	}
	new_rel_type->next = *list_head;
	if(*list_head!=NULL){
		(*list_head)->prec = new_rel_type;
	}
	new_rel_type->prec = NULL;
	*list_head = new_rel_type;
}

void ent_rel_list_head_insert(entity_relation_list* new_ent_rel, entity_relation_list** list_head){
	if(verbose){
		printf("Inserting relation from %s to %s of type %s to entity relation list at head of list pointed by %p ", new_ent_rel->rel->from->name, new_ent_rel->rel->to->name, new_ent_rel->rel_type->name , (void*)list_head);
		printf("(Old head of list was %s)\n", *list_head == NULL ? "NULL" : (*list_head)->name);
	}
	new_ent_rel->next = *list_head;
	if(*list_head!=NULL){
		(*list_head)->prec = new_ent_rel;
	}
	new_ent_rel->prec = NULL;
	*list_head = new_ent_rel;
}


entity* find_entity_in_list(entity* list_head, char* name){
	while(list_head!=NULL){
		if(strcmp(list_head->name, name)==0){
			if(verbose){
				printf("Found entity %s in list\n", name);
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

void print_entity_list(entity* list_head){
	while(list_head!=NULL){
		printf("Entity Name %s\n", list_head->name);
		list_head = list_head->next;
	}
}

void print_relation_list(relation* list_head){
	while(list_head!=NULL){
		printf("Relation from %s to %s\n", ((relation*)list_head)->from->name, ((relation*)list_head)->to->name);
		list_head = ((relation*)list_head)->next;
	}
}

void print_rel_type_list(relation_type* list_head){
	while(list_head!=NULL){
		printf("Relation Type: %s\n", ((relation_type*)list_head)->name);
		list_head = ((relation_type*)list_head)->next;
	}
}

void print_entity_relation_list(entity_relation_list* list_head){
	while(list_head!=NULL){
		printf("Relation from %s to %s\n", ((entity_relation_list*)list_head)->relation->from->name, ((entity_relation_list*)list_head)->relation->to->name);
		list_head = ((entity_relation_list*)list_head)->next;
	}

}

void entity_hash_table_insert(entity** hash_table, entity* ent, unsigned int position){
	ent_list_head_insert(ent, &hash_table[position]); 
}

void relation_hash_table_insert(relation** hash_table, relation* rel, unsigned int position){
	rel_list_head_insert(rel, &hash_table[position]);
}

entity* find_entity(entity** hash_table, char* name){
	unsigned int position = hash(string_compactor(1, name), ENTITY);
	return find_entity_in_list(hash_table[position], name);
}

relation** get_new_rel_hash_table(){
	//Creates a new Relation Hash Table, initializes it to NULL and returns a pointer to said table
	relation** hash_table_ptr= calloc(REL_HASH_TABLE_SIZE, sizeof(*hash_table_ptr));
	return hash_table_ptr;
}

entity** get_new_entity_hash_table(){
	//Creates a new Entity Hash Table, initializes it to NULL and returns a pointer to said table.
	entity** hash_table_ptr = calloc(ENTITY_HASH_TABLE_SIZE, sizeof(*hash_table_ptr));
	return hash_table_ptr;
}

rb_node** get_new_scoreboard_hash_table(){
	rb_node** hash_table_ptr = calloc(SCOREBOARD_HASH_TABLE_SIZE, sizeof(*hash_table_ptr));
	return hash_table_ptr;
}

void print_entity_hash_table(entity** hash_table, size_t hash_table_size){
	int i;
	for(i=0; i<hash_table_size; i++){
		if(hash_table[i]==NULL){
			if(suppress_NULL){
				continue;
			}
			printf("Position: %d - Content: NULL\n", i);
		} else {
			printf("Position: %d - Non empty cell: printing list...\n", i);
			print_entity_list(hash_table[i]);
		}
	}
}

void print_relation_hash_table(relation** hash_table, size_t hash_table_size){
	int i;
	for(i=0; i<hash_table_size; i++){
		if(hash_table[i]==NULL){
			if(suppress_NULL){
				continue;
			}
			printf("Position: %d - Content: NULL\n", i);
		} else {
			printf("Position: %d - Non empty cell: printing list...\n", i);
			print_relation_list(hash_table[i]);
		}
	}
}

void print_all_relations(relation_type* global_relation_type_list){
	while(global_relation_type_list != NULL){
		printf("Printing all relations of type %s\n", global_relation_type_list->name);
		print_relation_hash_table(global_relation_type_list->hash_table, REL_HASH_TABLE_SIZE);
		global_relation_type_list = global_relation_type_list->next;
	}
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
	va_list arglist;
	int i, j;
	char* currString;
	unsigned int currSum = 0;
	va_start(arglist, num);
	for(i=0; i<num; i++){
		currString = va_arg(arglist, char*);	
		for(j=0; j<strlen(currString); j++){
			currSum = currSum*33 + (int)currString[i];
		}
	}
	return currSum;
}

unsigned int hash(unsigned int key, data_t type) {
	//Not really a hash, more like a function to avoid any 
	switch(type){
		case ENTITY:
			return key%ENTITY_HASH_TABLE_SIZE;
		case REL:
			return key%REL_HASH_TABLE_SIZE;
		case SCOREBOARD:
			return key%SCOREBOARD_HASH_TABLE_SIZE;
		default:
			return 0;
	}
}

entity* add_new_entity(entity** hash_table, char* name){
	entity* new_entity = malloc(sizeof(entity));
	new_entity->name = malloc(strlen(name)+1);
	strcpy(new_entity->name, name);
	unsigned int position = hash(string_compactor(1,name), ENTITY);
	entity_hash_table_insert(hash_table, new_entity, position);	
	return new_entity;
}

relation* add_new_relation(entity* from, entity* to, relation_type* current_rel_type){
	relation** rel_hash_table = current_rel_type->hash_table;
	relation* new_relation = malloc(sizeof(relation));
	new_relation->from = from;
	new_relation->to = to;
	unsigned int position = hash(string_compactor(3, current_rel_type->name, from->name, to->name), REL);
	relation_hash_table_insert(rel_hash_table, new_relation, position); 
	return new_relation;
}

relation_type* add_new_relation_type(relation_type** relation_type_list, char* name){
	relation_type* new_rel_type = malloc(sizeof(relation_type));
	new_rel_type->name = malloc(strlen(name)+1);
	strcpy(new_rel_type->name, name);
	rel_type_list_head_insert(new_rel_type, relation_type_list);
	initialize_new_rel_type(new_rel_type);
	return new_rel_type;
}

entity_relation_list* add_relation_to_entities(entity* entity1, entity* entity2, relation_type* current_rel_type, relation* new_relation){
	entity_relation_list* new_entity_relation = malloc(sizeof(entity_relation_list));
	new_entity_relation->rel_type = current_rel_type;
	new_entity_relation->rel = new_relation;
	ent_rel_list_head_insert(new_entity_relation, entity1->rel_list);		
	if(strcmp(entity1->name, entity2->name)!=0){
		ent_rel_list_head_insert(new_entity_relation, entity2->rel_list);
	}
	return new_entity_relation;
}

void initialize_global_structure(entity*** hash_table, relation_type** relation_type_list){
	//Initializes every data structure necessary for the program
	*hash_table = get_new_entity_hash_table();
	*relation_type_list = NULL;
}

void initialize_rb_tree(rb_tree* tree){
	tree->root = NULL;
	tree->size = 0;
}

void initialize_new_rel_type(relation_type* new_rel_type){
	new_rel_type->hash_table = get_new_rel_hash_table();
	new_rel_type->scoreboard.hash_table = get_new_scoreboard_hash_table();
	initialize_rb_tree(&(new_rel_type->scoreboard.ent_tree));	
}

void addent(entity** global_entity_hash_table, char* name){
	if(!find_entity(global_entity_hash_table, name)){
		add_new_entity(global_entity_hash_table, name);
	}
}

void delent(){

}

void addrel(entity** global_entity_hash_table, relation_type** global_relation_type_list, char* from, char* to, char* name){
	entity* ent_from = find_entity(global_entity_hash_table, from);
	entity* ent_to = find_entity(global_entity_hash_table, to);
	if(ent_from && ent_to){
		relation_type* current_rel_type;
		if(!find_relation_type_in_list(*global_relation_type_list, name)){
			current_rel_type = add_new_relation_type(global_relation_type_list, name);	
		} else {
			current_rel_type = find_relation_type_in_list(*global_relation_type_list, name);
		}
		relation* new_relation = add_new_relation(ent_from, ent_to, current_rel_type);
		add_relation_to_entities(entity* entity1, entity* entity2, current_rel_type, new_relation);
	}
}

void delrel(){
	
}

void report(){

}

void testSuite1(entity** global_entity_hash_table, relation_type* global_relation_type_list){
	addent(global_entity_hash_table, "Giorgio");
	addent(global_entity_hash_table, "Gianni");
	addent(global_entity_hash_table, "Giorgdnskjaio");
	addent(global_entity_hash_table, "Giorgio2");
	addent(global_entity_hash_table, "Giorgio3");
	addent(global_entity_hash_table, "Giorgio43");
	addent(global_entity_hash_table, "Giorgio5kj");
	addent(global_entity_hash_table, "Giorgio232");
	addent(global_entity_hash_table, "Giorgio9");
	addent(global_entity_hash_table, "GiorgioGianni");
	addent(global_entity_hash_table, "GiorgioNove");
	addent(global_entity_hash_table, "GiorgioLol");
	print_entity_hash_table(global_entity_hash_table, ENTITY_HASH_TABLE_SIZE);
	printf("Finding Giorgio43: %s\n", find_entity(global_entity_hash_table, "Giorgio43")->name);
	printf("Finding inexistant entity: %p\n", (void*)find_entity(global_entity_hash_table, "Kien"));
	addrel(global_entity_hash_table, &global_relation_type_list, "Giorgio", "GiorgioLol", "killed");
	addrel(global_entity_hash_table, &global_relation_type_list, "Giorgio3", "Giorgio9", "is_in_a_relationship_with");
	addrel(global_entity_hash_table, &global_relation_type_list, "GiorgioGianni", "GiorgioGianni", "is_the_same_person_as");
	print_all_relations(global_relation_type_list);
}

int main(){
	entity** global_entity_hash_table;
	relation_type* global_relation_type_list;
	initialize_global_structure(&global_entity_hash_table, &global_relation_type_list);
	testSuite1(global_entity_hash_table, global_relation_type_list);
}

