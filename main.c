/*
 * ======================================================================================================================================
 *
 *       Filename:  progettoAPI2019.c 
 *
 *    Description:  My submission for the final project of Algorithms and Data Structures (Algoritmi e Strutture Dati) for the course
 *    		    of Algorithms and Foundations of Computer Science (Algoritmi e Principi dell'Informatica) at PoliMi, A.Y. 2018-2019.  
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

// VARIOUS CONSTANTS

#define REL_HASH_TABLE_SIZE 3000
#define ENTITY_HASH_TABLE_SIZE 1000 
#define MAX_HASH_VALUE UINT_MAX

#define INPUT_BUFFER_SIZE 400
#define OUTPUT_BUFFER_SIZE 6000
#define LINE_BUFFER_SIZE 5000

#define STEPS_BETWEEN_CLEANUPS 10000

//Set this constant to 0 to void verbose log messages
const int verbose = 0;

//Set this constant to 0 to skip over NULL cells when printing hash tables
const int suppress_NULL = 1;

// GLOBAL VARIABLES

int cleanup_needed = 0;
int update_needed = 1;


//TODO: Check for failed mallocs on all functions
//TODO: Rehashing when load factor is greater than 0.75

// STRUCTURES DECLARATIONS

struct ent;		//Entity structure.
struct rel;		//Relation structure: contains pointer information about the two entities concerned.
struct rel_type;	//Relation Type structure: contains the type of relation.
struct rb_node;		//Node of a Red-Black Tree: used for scoreboard purposes. Each node contains both a score (the key by which the tree is ordered) and the
			//name of the entity concerned.
struct rb_tree;		//Red-Black Tree structure: holds both the root node of the tree and a size variable.
struct scoreboard;	//Scoreboard structure: holds a rb_tree structure and a hash table to allow for quick retrieval of nodes based on entity names.

typedef struct ent{
	char* name;
	unsigned short int n_rel;
	struct rb_node* scoreboard_entry_list;
	struct ent* next;
	struct ent* prec;
} entity;

typedef struct rel{
	entity* from;
	entity* to;
	struct rel *p, *left, *right;
} relation;

typedef struct rb_node{
	//Order relation is greatest score first and alphanumerical in cases for which two nodes have the same score.
	unsigned short int score;	
	char colour;	
	entity* ent;
	struct rel_type* rel_type;
	struct rb_node* p;
	struct rb_node* left;
	struct rb_node* right;
	struct rb_node* next;
} rb_node;

typedef struct rel_type{
	char* name;
	struct rel_type* prec;
	struct rel_type* next;
	relation* relation_root;
	rb_node* root;
} relation_type;

typedef enum data_type{
	ENTITY,
	REL,
	REL_TYPE,
	ENT_REL_LIST
} data_t;

// CACHE VARIABLES

char* old_report = NULL;
char* output_string;  
char* current_line; 

entity* cached_entity_A = NULL;
entity* cached_entity_B = NULL;
relation_type* cached_relation_type = NULL;

// FUNCTION PROTOTYPES

// Linked Lists utility functions

// Insertion
void ent_list_head_insert(entity* new_entity, entity** list_head);
void rel_list_head_insert(relation* new_relation, relation** list_head);
void rel_type_list_head_insert(relation_type* new_rel_type, relation_type** list_head);
//void ent_rel_list_head_insert(entity_relation_list* new_ent_rel, entity_relation_list** list_head);

// Search
entity* find_entity_in_list(entity* list_head, char* name);
relation* find_relation_in_list(relation* list_head, char* from, char* to);
relation_type* find_relation_type_in_list(relation_type* list_head, char* name);

// Print
void print_entity_list(entity* list_head);
void print_relation_list(relation* list_head);
void print_rel_type_list(relation_type* list_head);
//void print_entity_relation_list(entity_relation_list* list_head);

// Hash Tables utility functions

// Insertion
void entity_hash_table_insert(entity**, entity*, unsigned int);
void relation_hash_table_insert(relation**, relation*, unsigned int);

// Creation
entity** get_new_entity_hash_table();
relation** get_new_rel_hash_table();

// Print
void print_entity_hash_table(entity** hash_table, size_t hash_table_size);
void print_relation_hash_table(relation** hash_table, size_t hash_table_size);
entity* find_entity(entity**, char*);
unsigned int hash(unsigned int, data_t);
unsigned int string_compactor(int, ...);
void initialize_new_rel_type(relation_type*);

// RB Trees utility functions

// Print
void rb_preorder_tree_walk(rb_node*);
void rb_inorder_tree_walk(rb_node*);

relation* relation_tree_search(relation* node, char* from, char* to, relation** insertion_point);

void ent_list_head_insert(entity* new_entity, entity** list_head){
	//Inserts a relation as head of list_head
	if(verbose){
		printf("[DEBUG] ");
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

void rel_type_list_head_insert(relation_type* new_rel_type, relation_type** list_head){
	//Inserts a relation type as head of list_head
	if(verbose){
		printf("[DEBUG] ");
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

void rel_type_list_ordered_insert(relation_type* new_rel_type, relation_type** list_head){
	//Inserts a relation type in the correct position
	if(verbose){
		printf("[DEBUG] Inserting (in order) relation type %s at head of list pointed by %p \n", new_rel_type->name, (void*) list_head);
	}
	relation_type* nav = *list_head;
	relation_type* follow = NULL;
	//nav is the pointer that leads the search
	//follow is the pointer one node behind it
	//When the search is finished we insert between nav and follow
	while(nav){
		if(strcmp(nav->name, new_rel_type->name) > 0){
			break;
		} else {
			follow = nav;
			nav = nav->next;
		}
	}
	if(follow){
		follow->next = new_rel_type;
	} else {
		//If follow == NULL then we're doing a head insert
		*list_head = new_rel_type;
	}
	if(nav){
		nav->prec = new_rel_type;
	}
	new_rel_type->prec = follow;
	new_rel_type->next = nav;
}

void rb_node_list_head_insert(rb_node* new_node, rb_node** list_head){
	new_node->next = *list_head;
	*list_head = new_node;
}

void entity_list_delete(entity* to_remove, entity** list_head){
	//Deletes to_remove entity from list
	if(verbose){
		printf("[DEBUG] ");
		printf("Deleting entity %s in list pointed by %p \n", to_remove->name, (void*)list_head);
	}
	if(to_remove->next){
		to_remove->next->prec = to_remove->prec;
	}
	if(to_remove->prec){
		to_remove->prec->next = to_remove->next;
	} else {
		*list_head = to_remove->next;
	}
	free(to_remove->name);
	free(to_remove);
}

void rb_node_list_delete(rb_node* to_remove, rb_node** list_head){
	rb_node* nav = *list_head;
	rb_node* follow = NULL;
	while(nav){
		if(nav == to_remove){
			if(follow){
				follow->next = nav->next;
			} else {
				*list_head = nav->next;
			}
		}
		follow = nav;
		nav = nav->next;
	}
}

entity* find_entity_in_list(entity* list_head, char* name){
	while(list_head!=NULL){
		if(strcmp(list_head->name, name)==0){
			if(verbose){
				printf("[DEBUG] ");
				printf("Found entity %s in list\n", name);
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
				printf("[DEBUG] ");
				printf("Found relation type in list\n");
			}
			return list_head;
		}
		list_head = list_head->next;
	}
	return NULL;
}

rb_node* find_scoreboard_entry_in_list(rb_node* list_head, relation_type* current_rel_type){
	while(list_head!=NULL){
		if(list_head->rel_type == current_rel_type){ 
			if(verbose){
				printf("[DEBUG] ");
				printf("Found scoreboard entry in list\n");
			}
			return list_head;
		}
		list_head = list_head->next;
	}
	return NULL;
}	

relation* find_relation(relation_type* current_rel_type, char* from, char* to, relation** insertion_point){
	relation* current_relation = NULL;
	if(current_rel_type->relation_root){
		current_relation = relation_tree_search(current_rel_type->relation_root, from, to, insertion_point);	
	}
	return current_relation;
}

void print_entity_list(entity* list_head){
	while(list_head!=NULL){
		printf("[PRINT] (%p) ", (void*)list_head);
		printf("Entity Name %s\n", list_head->name);
		list_head = list_head->next;
	}
}

void print_rel_type_list(relation_type* list_head){
	while(list_head!=NULL){
		printf("[PRINT] (%p) ", (void*)list_head);
		printf("Relation Type: %s\n", list_head->name);
		list_head = ((relation_type*)list_head)->next;
	}
}

void entity_hash_table_insert(entity** hash_table, entity* ent, unsigned int position){
	ent_list_head_insert(ent, &hash_table[position]); 
}

entity* find_entity(entity** hash_table, char* name){
	unsigned int position = hash(string_compactor(1, name), ENTITY);
	return find_entity_in_list(hash_table[position], name);
}
  
rb_node* find_scoreboard_entry(relation_type* current_rel_type, entity* ent){
	return find_scoreboard_entry_in_list(ent->scoreboard_entry_list, current_rel_type);
}

entity** get_new_entity_hash_table(){
	//Creates a new Entity Hash Table, initializes it to NULL and returns a pointer to said table.
	entity** hash_table_ptr = calloc(ENTITY_HASH_TABLE_SIZE, sizeof(*hash_table_ptr));
	return hash_table_ptr;
}

void print_entity_hash_table(entity** hash_table, size_t hash_table_size){
	int i;
	for(i=0; i<hash_table_size; i++){
		if(hash_table[i]==NULL){
			if(suppress_NULL){
				continue;
			}
			printf("[PRINT] (%p) ", (void*)hash_table);
			printf("Position: %d - Content: NULL\n", i);
		} else {
			printf("[PRINT] (%p) ", (void*)hash_table);
			printf("Position: %d - Non empty cell: printing list...\n", i);
			print_entity_list(hash_table[i]);
		}
	}
}

void print_all_scoreboard_entries(relation_type* global_relation_type_list){
	while(global_relation_type_list != NULL){
		printf("[PRINT] ");
		printf("Printing the scoreboard for relation of type %s\n", global_relation_type_list->name);
		printf("Printing the tree: \n");
		rb_preorder_tree_walk(global_relation_type_list->root);
		global_relation_type_list = global_relation_type_list->next;
	}
}

int relation_node_compare(relation* x, relation* y){
	if(x==NULL){
		return 0;
	}
	if(y==NULL){
		return 1;
	}
	if(strcmp(x->to->name, y->to->name) > 0){ //If the name of x's receiver comes after y's receiver then x>y
		return 1;
	} else if (strcmp(x->to->name, y->to->name) < 0){ //If the name of y's receiver comes after then x<y
		return -1;
	} else { //If names are the same then compare by name of sender (As there can't be duplicates, this covers all cases)
		return strcmp(x->from->name, y->from->name) < 0 ? -1 : 1;
	}
}

int relation_node_string_compare(relation* x, char* from, char* to){
	int a = strcmp(x->from->name, from);
	int b = strcmp(x->to->name, to);
	if(a==0 && b==0){
		return 0;
	}
	if(x==NULL){
		return -1;
	}
	if(b  > 0){
		return 1;
	} else if (b < 0){
		return -1;
	} else {
		return a < 0 ? -1 : 1;
	}
}

relation* relation_tree_search(relation* node, char* from, char* to, relation** insertion_point){
	if(relation_node_string_compare(node, from, to)==0){
		return node;
	}
	if(relation_node_string_compare(node, from, to) > 0){
		if(node->left == NULL){
			if(insertion_point){
				*insertion_point = node;	
			}
			return NULL;
		} else{
			return relation_tree_search(node->left, from, to, insertion_point);
		}
	} else {
		if(node->right == NULL){
			if(insertion_point){
				*insertion_point = node;	
			}
			return NULL;
		} else{
			return relation_tree_search(node->right, from, to, insertion_point);
		}
	}
}

relation* relation_tree_minimum(relation* z){
	while(z->left){
		z = z->left;
	}
	return z;
}

relation* relation_tree_successor(relation* z){
	if(z->right != NULL){
		return relation_tree_minimum(z->right);
	}
	relation* y = z->p;
	while(y && z == y->right){
		z = y;
		y = y->p;
	}
	return y;
}

void relation_tree_precision_insert(relation** root, relation* z, relation* insertion_point){
	//Acts like relation_tree_insert but uses the additional information from a precedent search to know already where the node needs to be inserted
	z->p = insertion_point;
	if(insertion_point == NULL){
		*root = z;
	} else if(relation_node_compare(z, insertion_point)<0){
		insertion_point->left = z;
	} else {
		insertion_point->right = z;
	}
	z->left = NULL;
	z->right = NULL;
}

void relation_tree_insert(relation** root, relation* z){
	relation* y = NULL;
	relation* x = *root;
	while(x){
		y = x;
		if(relation_node_compare(z, x)<0){
			x = x->left;
		} else {
			x = x->right;
		}
	}
	z->p = y;
	if(y == NULL){
		*root = z;
	} else if(relation_node_compare(z, y)<0){
		y->left = z;
	} else {
		y->right = z;
	}
	z->left = NULL;
	z->right = NULL;
}

void relation_tree_delete(relation** root, relation* z){
	relation* x = NULL;
	relation* y = NULL;
	if(z->left == NULL || z->right == NULL){
		y = z;
	} else {
		y = relation_tree_successor(z);
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
		*root = x;
	} else if(y == y->p->left){
		y->p->left = x;
	} else {
		y->p->right = x;
	}
	if(y != z){
		z->from = y->from;
		z->to = y->to;
	}
	free(y);
}

void relation_preorder_tree_walk(relation* node){
	if(node){
		printf("[PRINT] ");
		printf("Printing relation in tree from %s to %s\n", node->from->name, node->to->name);
		relation_preorder_tree_walk(node->left);
		relation_preorder_tree_walk(node->right);	
	} else {
		printf("[PRINT] ");
		printf("Printing NULL node\n");
	}
}

void print_all_relation_trees(relation_type* global_relation_type_list){
	while(global_relation_type_list){
		if(global_relation_type_list->relation_root){
			printf("[PRINT] ");
			printf("Printing tree of relation type %s\n", global_relation_type_list->name);
			relation_preorder_tree_walk(global_relation_type_list->relation_root);
		} else {
			printf("[PRINT] ");
			printf("Tree for relation type %s is empty", global_relation_type_list->name);
		}
		global_relation_type_list = global_relation_type_list->next;
	}

}

int rb_node_compare(rb_node* x, rb_node* y){ //Acts as a > 'greater than' sign
	if(x==NULL){
		return 0;
	}
	if(y==NULL){
		return 1;
	}
	if(x->score > y->score){ //If the first one has greater score then x>y
		return 1;
	} else if (x->score < y->score){ //If the second one has greater score then x<y
		return 0;
	} else { //If the scores are the same then the one that comes before is considered the greater
		return strcmp(x->ent->name, y->ent->name) < 0 ? 1 : 0;
	}
}

void rb_left_rotate(rb_node** root, rb_node* x){
	rb_node* y = x->right;
	x->right = y->left;
	if(y->left != NULL){
		y->left->p = x;
	}	
	y->p = x->p;
	if(x->p == NULL){
		*root = y;
	}else if(x==x->p->left){
		x->p->left = y;
	}else{
		x->p->right = y;
	}
	y->left = x;
	x->p = y;
}

void rb_right_rotate(rb_node** root, rb_node* x){ 
	rb_node* y = x->left;
	x->left = y->right;
	if(y->right != NULL){
		y->right->p = x;
	}	
	y->p = x->p;
	if(x->p == NULL){
		*root= y;
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

rb_node* rb_tree_predecessor(rb_node* z){
	if(z->left != NULL){
		return rb_tree_maximum(z->left);
	}
	rb_node* y = z->p;
	while(y!=NULL && z == y->left){
		z = y;
		y = y->p;
	}
	return y;
}

void rb_insert_fixup(rb_node** root, rb_node* z){
	rb_node *x, *y;
	if(z == *root){
		(*root)->colour = 'b';
	}else{
		x = z->p;
		if(x->colour == 'r'){
			if(x == x->p->left){
				y = x->p->right;
				if(y!= NULL && y->colour == 'r'){
					x->colour = 'b';
					y->colour = 'b';
					x->p->colour = 'r';
					rb_insert_fixup(root, x->p);
				} else {
					if (z == x->right){
						z = x;
						rb_left_rotate(root, z);
						x = z->p;
					}
					x->colour = 'b';
					x->p->colour = 'r';
					rb_right_rotate(root, x->p);
				}
			} else {
				y = x->p->left;
				if(y != NULL && y->colour == 'r'){
					x->colour = 'b';
					y->colour = 'b';
					x->p->colour = 'r';
					rb_insert_fixup(root, x->p);
				} else {
					if (z == x->left){
						z = x;
						rb_right_rotate(root, z);
						x = z->p;
					}
					x->colour = 'b';
					x->p->colour = 'r';
					rb_left_rotate(root, x->p);
				}
			}
		}
	}
}

void rb_delete_fixup(rb_node** root, rb_node* x, rb_node* parent,  char side){
	rb_node* w;
	if(x!=NULL && (x->colour == 'r' || x->p == NULL)){
		x->colour = 'b';
	} else if(x==NULL && parent == NULL) {
		//Case in which x.p = NULL but x is also NULL, which means that the tree is empty
		return;
	} else if(side == 'l'){
		w = parent->right;
		if(w->colour == 'r'){
			w->colour = 'b';
			parent->colour = 'r';
			rb_left_rotate(root, parent);
			w = parent->right;
		}
		if((w->left == NULL || w->left->colour == 'b') && (w->right == NULL || w->right->colour == 'b')){
			w->colour = 'r';
			if(parent->p==NULL){
				//Side doesn't matter here, execution will stop after the first if because x->p = NULL
				rb_delete_fixup(root, parent, NULL, 'l');
			} else if(parent == parent->p->left){
				rb_delete_fixup(root, parent, parent->p, 'l');
			} else {
				rb_delete_fixup(root, parent, parent->p, 'r');
			}
		} else {
			if(w->right == NULL || w->right->colour == 'b'){
				w->left->colour = 'b';
				w->colour = 'r';
				rb_right_rotate(root, w);
				w = parent->right;
			}
			w->colour = parent->colour;
			parent->colour = 'b';
			w->right->colour = 'b';
			rb_left_rotate(root, parent);
		}
	} else if(side == 'r') {
		w = parent->left;
		if(w->colour == 'r'){
			w->colour = 'b';
			parent->colour = 'r';
			rb_right_rotate(root, parent);
			w = parent->left;
		}
		if((w->right == NULL || w->right->colour == 'b') && (w->left == NULL || w->left->colour == 'b')){
			w->colour = 'r';
			if(parent->p == NULL){
				rb_delete_fixup(root, parent, NULL, 'l');
			} else if(parent == parent->p->left){
				rb_delete_fixup(root, parent, parent->p, 'l');
			} else {
				rb_delete_fixup(root, parent, parent->p, 'r');
			}
		} else {
			if(w->left == NULL || w->left->colour == 'b'){
				w->right->colour = 'b';
				w->colour = 'r';
				rb_left_rotate(root, w);
				w = parent->left;
			}
			w->colour = parent->colour;
			parent->colour = 'b';
			w->left->colour = 'b';
			rb_right_rotate(root, parent);
		}
	} else {
		printf("Invalid Side\n");
		return;
	}
}

void rb_delete(rb_node** root, rb_node* z){
	//Deletes node and returns its key
	rb_node* y = NULL;
	rb_node* x = NULL;
	char side;
	if(z->left == NULL || z->right == NULL){
		 y = z;
	} else {
		y = rb_tree_successor(z);		
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
		*root = x;
	} else if (y == y->p->left){
		side = 'l';
		y->p->left = x;
	} else {
		side = 'r';
		y->p->right = x;
	}
	rb_node* parent = y->p;
	char old_colour = y->colour;
	if (y != z){
		//We need to swap the nodes entirely: just changing the values destroys the link between nodes and scoreboard entries
		if(y->p == z){
			//If parent of y was z then, after the exchange, the parent of x will be y itself.
			//The problem arises because otherwise y->p still points to z, which contains outdated information
			parent = y;
		}
		y->colour = z->colour;
		if(z->p == NULL){
			*root = y;
		}
		y->p = z->p;
		if(y->p){
			if(z == z->p->left){
				y->p->left = y;
			} else {
				y->p->right = y;
			}
		}
		y->left = z->left;
		y->right = z->right;
		if(y->left){
			y->left->p = y;
		}
		if(y->right){
			y->right->p = y;
		}
	}
	if (old_colour == 'b'){	
		rb_delete_fixup(root, x, parent, side);
	}
	z->left = NULL;
	z->right = NULL;
	z->p = NULL;
}

void rb_insert(rb_node** root, rb_node* z){
	rb_node* y = NULL;
	rb_node* x = *root;
	while(x!=NULL){
		y = x;
		if(!rb_node_compare(z, x)){
			x = x->left;
		} else {
			x = x->right;
		}
	}	
	z->p = y;
	if(y == NULL){
		*root = z;
	} else if(!rb_node_compare(z, y)) {
		y->left = z;
	} else {
		y->right = z;
	}
	z->left = NULL;
	z->right = NULL;
	z->colour = 'r';
	rb_insert_fixup(root, z);
}

void rb_inorder_tree_walk(rb_node* node){
	if(node->left != NULL){
		rb_inorder_tree_walk(node->left);
	} else {
		printf("[PRINT] ");
		printf("Hit a NULL node\n");
	}	
	printf("[PRINT] ");
	printf("Printing node with score %d and colour %c\n", node->score, node->colour);
	if(node->right != NULL){
		rb_inorder_tree_walk(node->right);
	} else { 
		printf("[PRINT] ");
		printf("Hit a NULL node\n");
	}
}

void rb_modified_inorder_tree_walk(rb_node* node, char* ent_string, int score_to_search){
	if(node->right != NULL){
		rb_modified_inorder_tree_walk(node->right, ent_string, score_to_search);
	}
	if(node->score == score_to_search){
		strcat(ent_string, " \"");
		strcat(ent_string, node->ent->name);
		strcat(ent_string, "\"");
	} else {
		return;
	}
	if(node->left != NULL){
		rb_modified_inorder_tree_walk(node->left, ent_string, score_to_search);
	}
}

void rb_preorder_tree_walk(rb_node* node){
	printf("[PRINT] ");
	printf("Printing node with score %d and colour %c for entity %s\n", node->score, node->colour, node->ent->name);
	if(node->left != NULL){
		rb_preorder_tree_walk(node->left);
	} else {
		printf("[PRINT] ");
		printf("Hit a NULL node\n");
	}	
	if(node->right != NULL){
		rb_preorder_tree_walk(node->right);
	} else { 
		printf("[PRINT] ");
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
		for(j=0; j<strlen(currString); j+=2){
			currSum = currSum*33 + (int)currString[j];
		}
	}
	return currSum;
}

unsigned int hash(unsigned int key, data_t type) {
	switch(type){
		case ENTITY:
			return key%ENTITY_HASH_TABLE_SIZE;
		case REL:
			return key%REL_HASH_TABLE_SIZE;
		default:
			return 0;
	}
}

entity* add_new_entity(entity** hash_table, char* name){
	entity* new_entity = malloc(sizeof(entity));
	new_entity->name = malloc(strlen(name)+1);
	new_entity->scoreboard_entry_list = NULL;
	new_entity->n_rel = 0;
	strcpy(new_entity->name, name);
	unsigned int position = hash(string_compactor(1,name), ENTITY);
	if(verbose){
		printf("[DEBUG] ");
		printf("Adding new entity %s in position %d of hash table\n", name, position);
	}
	entity_hash_table_insert(hash_table, new_entity, position);	
	return new_entity;
}

relation* add_new_relation(entity* from, entity* to, relation_type* current_rel_type, relation* insertion_point){
	relation* new_relation = malloc(sizeof(relation));
	new_relation->from = from;
	new_relation->to = to;
	if(insertion_point){
		relation_tree_precision_insert(&(current_rel_type->relation_root), new_relation, insertion_point);
	} else {
		relation_tree_insert(&(current_rel_type->relation_root), new_relation);
	}
	return new_relation;
}

relation_type* add_new_relation_type(relation_type** relation_type_list, char* name){
	relation_type* new_rel_type = malloc(sizeof(relation_type));
	new_rel_type->name = malloc(strlen(name)+1);
	strcpy(new_rel_type->name, name);
	rel_type_list_ordered_insert(new_rel_type, relation_type_list);
	initialize_new_rel_type(new_rel_type);
	return new_rel_type;
}

void delete_scoreboard_entry(rb_node* current_entry, relation_type* current_rel_type){
	rb_delete(&(current_rel_type->root), current_entry);
	rb_node_list_delete(current_entry, &(current_entry->ent->scoreboard_entry_list));
	free(current_entry);
}

rb_node* add_entry_to_scoreboard(entity* ent, relation_type* current_rel_type, int score){
	rb_node* new_node = malloc(sizeof(rb_node));
	new_node->score = score;
	new_node->ent = ent;
	new_node->rel_type = current_rel_type;
	rb_node_list_head_insert(new_node, &(ent->scoreboard_entry_list));
	rb_insert(&(current_rel_type->root), new_node);
	return new_node;
}

void update_scoreboard_entry(int score_difference, rb_node* current_entry, relation_type* current_rel_type){
	if(current_entry->score + score_difference <= 0){
		delete_scoreboard_entry(current_entry, current_rel_type);
	} else {
		rb_delete(&(current_rel_type->root), current_entry);
		current_entry->score += score_difference;
		rb_insert(&(current_rel_type->root), current_entry);
	}
}

void delete_entity(entity** global_entity_hash_table, entity* to_delete){
	if(to_delete->prec == NULL){
		unsigned int position = hash(string_compactor(1, to_delete->name), ENTITY);
		entity_list_delete(to_delete, &(global_entity_hash_table[position]));
	} else {
		to_delete->prec->next = to_delete->next;
		if(to_delete->next){
			to_delete->next->prec = to_delete->prec;
		}
		free(to_delete->name);
		free(to_delete);
	}
}

void delete_relation(relation_type* current_rel_type, relation* to_delete){
	/*  
	unsigned int position = hash(string_compactor(3, current_rel_type->name, to_delete->from->name, to_delete->to->name), REL);
	relation_list_delete(to_delete, &(current_rel_type->hash_table[position]));
	*/
	relation_tree_delete(&(current_rel_type->relation_root), to_delete);
}

void relation_type_list_delete(relation_type** global_relation_type_list, relation_type* to_delete){
	relation_type* nav = *global_relation_type_list;
	while(nav){
		if(strcmp(nav->name, to_delete->name)==0){
			if(nav->next){
				nav->next->prec = nav->prec;
			}
			if(nav->prec){
				nav->prec->next = nav->next;
			} else {
				*global_relation_type_list = nav->next;
			}
			//free(nav->hash_table); //Assuming that the hash_table is already empty
			free(nav);
			return;
		}
		nav = nav->next;
	}
}

void update_entity_relation_count(unsigned int score_difference, rb_node* rb_node_list, relation_type* current_rel_type){
	while(rb_node_list){
		if(rb_node_list->rel_type == current_rel_type){
			update_scoreboard_entry(score_difference, rb_node_list, current_rel_type);
			return;
		}
		rb_node_list = rb_node_list->next;
	}
}

void initialize_global_structure(entity*** hash_table, relation_type** relation_type_list){
	//Initializes every data structure necessary for the program
	old_report = malloc(OUTPUT_BUFFER_SIZE);
	*old_report = '\0';
	current_line = malloc(LINE_BUFFER_SIZE);
	output_string = malloc(OUTPUT_BUFFER_SIZE);
	*hash_table = get_new_entity_hash_table();
	*relation_type_list = NULL;
}

void delete_empty_rel_types(relation_type** relation_type_list){
	relation_type* nav = *relation_type_list;
	while(nav){
		if(nav->root == NULL){
			//If tree is now empty, deallocate the relation type object
			relation_type_list_delete(relation_type_list, nav);
		}
		nav = nav->next;
	}
}

void clear_invalid_relations_in_tree(relation* node, relation** root, relation_type* current_rel_type, entity* to_delete){
	//Does a post-order walk of the tree and deletes any node that has one (or more) of its entities in global_deleted_entities_list
	if(node){
		clear_invalid_relations_in_tree(node->left, root, current_rel_type,  to_delete);
		if(to_delete->n_rel == 0){
			return;
		}
		clear_invalid_relations_in_tree(node->right, root, current_rel_type, to_delete);
		if(to_delete->n_rel == 0){
			return;
		}
		if(node->from == to_delete || node->to == to_delete){
			if(node->to != to_delete){
				update_entity_relation_count(-1, node->to->scoreboard_entry_list, current_rel_type);
			}
			if(node->to != node->from){
				node->to->n_rel += -1;
			}
			node->from->n_rel += -1;
			relation_tree_delete(root, node);
		}
	}
}

//void clear_invalid_entries(entity** global_entity_hash_table, relation_type** rel_list, deleted_entity** global_deleted_entities_list){
void clear_invalid_entries(entity** global_entity_hash_table, relation_type** rel_list, entity* to_delete){
	relation_type* nav = *rel_list;
	while(to_delete->n_rel != 0 && nav){
		//clear_invalid_relations_in_tree(nav->relation_root, &(nav->relation_root), nav, *global_deleted_entities_list);
		clear_invalid_relations_in_tree(nav->relation_root, &(nav->relation_root), nav, to_delete);
		nav = nav->next;
	}
}

void cleanup(entity*** hash_table, relation_type** relation_type_list){
	//Clears every data structure at program end	
}

void initialize_new_rel_type(relation_type* new_rel_type){
	//new_rel_type->hash_table = get_new_rel_hash_table();
	new_rel_type->relation_root = NULL;
	new_rel_type->root = NULL;
}

char* string_concatenate(char* dest, char* src){
	//Better strcat, taken from Joel Spolsky's "Back to Basics" post
	while(*dest){
		dest++;
	}
	while((*dest++ = *src++));
	return --dest;
}

void addent(entity** global_entity_hash_table, relation_type** global_relation_type_list,  char* name){
	//Expected complexity O(1)
	if(verbose){
		printf("[DEBUG] ");
		printf("addent: entity %s\n", name);
	}
	entity* current_entity;
	if(cached_entity_A && strcmp(cached_entity_A->name, name)==0){
		if(verbose){
			printf("[DEBUG] ");
			printf("Using cached entity %s\n", name); 
		}
		current_entity = cached_entity_A;	
	} else {
		current_entity = find_entity(global_entity_hash_table, name);
		if(current_entity){
			cached_entity_A = current_entity;
		}
	}
	if(!current_entity){
		//If entity does not already exist then add it
		add_new_entity(global_entity_hash_table, name);
	}
}

void delent(entity** global_entity_hash_table, relation_type** global_relation_type_list, char* name){
	if(verbose){
		printf("[DEBUG] ");
		printf("delent: entity %s\n", name);
	}
	//Check if entity actually exists and tries to fetch it
	entity* current_entity;
	if(cached_entity_A && strcmp(cached_entity_A->name, name)==0){
		if(verbose){
			printf("[DEBUG] ");
			printf("Using cached entity %s\n", name); 
		}
		current_entity = cached_entity_A;	
		cached_entity_A = NULL;
	} else {
		current_entity = find_entity(global_entity_hash_table, name);
	}
	if(cached_entity_B && strcmp(cached_entity_B->name, name)==0){
		cached_entity_B = NULL;
	}
	if(current_entity){
		//If the entity exists, then delete every reference to this entity from other entities
		relation_type* nav = *global_relation_type_list;
		while(nav){
			//For each relation type, find and delete the scoreboard entry (if it exists) associated with the entity
			rb_node* current_scoreboard_entry = find_scoreboard_entry(nav, current_entity);
			if(current_scoreboard_entry){
				delete_scoreboard_entry(current_scoreboard_entry, nav);	
			}
			nav = nav->next;
		}
		//Clear up the entity
		clear_invalid_entries(global_entity_hash_table, global_relation_type_list, current_entity);
		delete_entity(global_entity_hash_table, current_entity);
		update_needed = 1;
	}
}

void addrel(entity** global_entity_hash_table, relation_type** global_relation_type_list, char* from, char* to, char* name){
	//Expected complexity O(log(e)) 
	if(verbose){
		printf("[DEBUG] ");
		printf("addrel: from %s to %s. Type: %s\n", from, to, name);
	}
	//Find if both entities that are involved exist
	entity *ent_from = NULL, *ent_to = NULL;
	relation* insertion_point = NULL;
	if(cached_entity_A){
		if(strcmp(cached_entity_A->name, from)==0){
			if(verbose){
				printf("[DEBUG] ");
				printf("Using cached entity A as from: %s\n", cached_entity_A->name); 
			}
			ent_from = cached_entity_A;
		}
		if(strcmp(cached_entity_A->name, to)==0){
			if(verbose){
				printf("[DEBUG] ");
				printf("Using cached entity A as to: %s\n", cached_entity_A->name); 
			}
			ent_to = cached_entity_A;
		}
	}
	if(cached_entity_B && (!ent_from || !ent_to)){
		if(strcmp(cached_entity_B->name, from)==0){
			if(verbose){
				printf("[DEBUG] ");
				printf("Using cached entity B as from: %s\n", cached_entity_B->name); 
			}
			ent_from = cached_entity_B;
		}
		if(strcmp(cached_entity_B->name, to)==0){
			if(verbose){
				printf("[DEBUG] ");
				printf("Using cached entity B as to: %s\n", cached_entity_B->name); 
			}
			ent_to = cached_entity_B;
		}
	}
	if(!ent_from) {
		ent_from  = find_entity(global_entity_hash_table, from);
		if(ent_from){
			if(verbose){
				printf("[DEBUG] ");
				printf("Overwriting cached entity A with %s\n", ent_from->name); 
			}
			cached_entity_A = ent_from;
		}
	}
	if(!ent_to){
		ent_to  = find_entity(global_entity_hash_table, to);
		if(ent_to){
			if(verbose){
				printf("[DEBUG] ");
				printf("Overwriting cached entity B with %s\n", ent_to->name); 
			}
			cached_entity_B = ent_to;
		}
	}
	if(ent_from && ent_to){
		//If both entities actually exist then attempt an insertion of a new relation between ent_from and ent_to
		relation_type* current_rel_type;
		if(cached_relation_type && strcmp(cached_relation_type->name, name)==0){
			if(verbose){
				printf("[DEBUG] ");
				printf("Using cached relation type: %s\n", cached_relation_type->name); 
			}
			current_rel_type = cached_relation_type;
		} else {
			current_rel_type = find_relation_type_in_list(*global_relation_type_list, name);
			if(current_rel_type){
				if(verbose){
					printf("[DEBUG] ");
					printf("Overwriting cached relation type with %s\n", current_rel_type->name); 
				}
				cached_relation_type = current_rel_type;
			}
		}
		if(!current_rel_type){
			//If the current relation type does not exist then create it
			current_rel_type = add_new_relation_type(global_relation_type_list, name);	
			//And create a new entry for the scoreboard
			add_entry_to_scoreboard(ent_to, current_rel_type, 1);
		} else {
			//If the current relation type does exist...
			if(find_relation(current_rel_type, from, to, &insertion_point)){
				//...check if the relation already exists...
				return;
			}
			//...if not, then check if there's an existing scoreboard entry for the receiving entity
			rb_node* current_scoreboard_entry = find_scoreboard_entry(current_rel_type, ent_to);
			if(current_scoreboard_entry){
				//If the scoreboard entry exists, then update it, adding 1 to its score
				update_scoreboard_entry(1, current_scoreboard_entry, current_rel_type);
			} else {
				//Otherwise create a new scoreboard entry with score 1
				add_entry_to_scoreboard(ent_to, current_rel_type, 1);
			}
		}
		if(ent_from != ent_to){
			ent_from->n_rel += 1;
		}
		ent_to->n_rel += 1;
		//Create the new relation object and add it to the hash table of relations
		add_new_relation(ent_from, ent_to, current_rel_type, insertion_point);
		//Also insert a reference to the newly created relation in the entity relation list
		update_needed = 1;
	}
}

void delrel(entity** global_entity_hash_table, relation_type** global_relation_type_list, char* from, char* to, char* name){
	//Check if relation type exists
	if(verbose){
		printf("[DEBUG] ");
		printf("delrel: from %s to %s. Type: %s\n", from, to, name);
	}
	relation_type* current_rel_type = find_relation_type_in_list(*global_relation_type_list, name);
	if(current_rel_type){
		entity *ent_from = NULL, *ent_to = NULL;
		if(cached_entity_A){
			if(strcmp(cached_entity_A->name, from)==0){
				ent_from = cached_entity_A;
			}
			if(strcmp(cached_entity_A->name, to)==0){
				ent_to = cached_entity_A;
			}
		}
		if(cached_entity_B && (!ent_from || !ent_to)){
			if(strcmp(cached_entity_B->name, from)==0){
				ent_from = cached_entity_B;
			}
			if(strcmp(cached_entity_B->name, to)==0){
				ent_to = cached_entity_B;
			}
		}
		if(!ent_from) {
			ent_from  = find_entity(global_entity_hash_table, from);
			if(ent_from){
				cached_entity_A = ent_from;
			}
		}
		if(!ent_to){
			ent_to  = find_entity(global_entity_hash_table, to);
			if(ent_to){
				cached_entity_B = ent_to;
			}
		}
		//If so, find both entities involved
		if(ent_from && ent_to){
			//If both entities exist, then check if the relation that we need to delete exists
			relation* current_relation = find_relation(current_rel_type, from, to, NULL);
			if(!current_relation){
				//If no such relation exists, then do nothing
				return;
			}
			//Otherwise we need to actually delete the relation: update the scoreboard, removing 1 from the score
			rb_node* current_scoreboard_entry = find_scoreboard_entry(current_rel_type, ent_to);
			update_scoreboard_entry(-1, current_scoreboard_entry, current_rel_type);
			//Delete the relation object
			delete_relation(current_rel_type, current_relation);
			if(ent_from != ent_to){
				ent_from->n_rel += -1;
			}
			ent_to->n_rel += -1;
			update_needed = 1;
		}
	}
}


void report(entity** global_entity_hash_table, relation_type** global_relation_type_list){
	delete_empty_rel_types(global_relation_type_list);
	if(!update_needed){
		puts(old_report);
		return;
	}
	relation_type* nav = *global_relation_type_list;
	*output_string = '\0';
	int first_line = 1;
	char points[12];
	while(nav){
		*current_line = '\0';
		if(!first_line){
			//Adding space after semicolon if needed
			strcat(current_line, " ");
		} else {
		}
		strcat(current_line, "\"");
		strcat(current_line, nav->name);
		strcat(current_line, "\"");	
		rb_node* max_node = rb_tree_maximum(nav->root);
		rb_modified_inorder_tree_walk(nav->root, current_line, max_node->score);
		sprintf(points, " %d;", max_node->score);
		strcat(current_line, points);
		strcat(output_string, current_line);
		nav = nav->next;
		if(first_line){
			first_line = 0;
		}
	}
	if(strlen(output_string)){
		puts(output_string);
		strcpy(old_report, output_string);
	} else {
		puts("none");
		strcpy(old_report, "none");
	}
	update_needed = 0;
}

int main(){
	entity** global_entity_hash_table;
	relation_type* global_relation_type_list;
	initialize_global_structure(&global_entity_hash_table, &global_relation_type_list);
	char *inputBuffer = malloc(INPUT_BUFFER_SIZE);
	size_t n = INPUT_BUFFER_SIZE;
	char *command = NULL, *arg1 = NULL, *arg2 = NULL, *arg3 = NULL;
	char separators[] = " \"\n";
	while(1){
		if(getline(&inputBuffer, &n, stdin)){
			if(inputBuffer[0]=='e'){ //This means that the command is [e]nd
				return 0;
			} 
			command = strtok(inputBuffer, separators);
			if(command[0]=='r'){
				report(global_entity_hash_table, &global_relation_type_list);
			} else if(command[0]=='a'){
				if(command[3]=='e'){
					arg1 = strtok(NULL, separators);	
					addent(global_entity_hash_table, &global_relation_type_list, arg1);
				} else if(command[3]=='r'){
					arg1 = strtok(NULL, separators);
					arg2 = strtok(NULL, separators);
					arg3 = strtok(NULL, separators);
					addrel(global_entity_hash_table, &global_relation_type_list, arg1, arg2, arg3);
				}
			} else if (command[0] == 'd'){
				if(command[3]=='e'){
					arg1 = strtok(NULL, separators);
					delent(global_entity_hash_table, &global_relation_type_list, arg1);
				} else if(command[3]=='r'){
					arg1 = strtok(NULL, separators);
					arg2 = strtok(NULL, separators);
					arg3 = strtok(NULL, separators);
					delrel(global_entity_hash_table, &global_relation_type_list, arg1, arg2, arg3);
				}
			}
			if(verbose){
				//print_all_scoreboards(global_relation_type_list);
			}
		} else {
			if(verbose){
				printf("Failed to read line: terminating program\n");
			}
		}
	}
	return 0;
}
