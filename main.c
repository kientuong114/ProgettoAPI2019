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

#define REL_HASH_TABLE_SIZE 5000
#define ENTITY_HASH_TABLE_SIZE 20000 //Should be more than enough
#define MAX_HASH_VALUE UINT_MAX
#define SCOREBOARD_HASH_TABLE_SIZE 5000 //Should be same to REL_HASH_TABLE_SIZE

#define MAX_HASH_MASK 0xFFFFFFFFFFFFFFFF //2^64
#define REL_HASH_TABLE_MASK 0x0 
#define ENTITY_HASH_TABLE_MASK 0x0
#define SCOREBOARD_HASH_TABLE_MASK 0x0

#define INPUT_BUFFER_SIZE 1000
#define OUTPUT_BUFFER_SIZE 1000
#define LINE_BUFFER_SIZE 200

//Set this constant to 0 to void verbose log messages
const int verbose = 0;

//Set this constant to 0 to skip over NULL cells when printing hash tables
const int suppress_NULL = 1;

//TODO: Check for failed mallocs on all functions
//TODO: Rehashing when load factor is greater than 0.75

// STRUCTURES DECLARATIONS

struct ent;		//Entity structure.
struct rel;		//Relation structure: contains pointer information about the two entities concerned.
struct rel_type;	//Relation Type structure: contains the type of relation.
struct ent_rel_list;	//Entity Relation List structure: holds a list of all relations concerning a single entity.
struct rb_node;		//Node of a Red-Black Tree: used for scoreboard purposes. Each node contains both a score (the key by which the tree is ordered) and the
			//name of the entity concerned.
struct rb_tree;		//Red-Black Tree structure: holds both the root node of the tree and a size variable.
struct scoreboard;	//Scoreboard structure: holds a rb_tree structure and a hash table to allow for quick retrieval of nodes based on entity names.

typedef struct ent{
	char* name;
	struct ent* next;
	struct ent* prec;
	struct ent_rel_list* rel_list;
} entity;

typedef struct rel{
	entity* from;
	entity* to;
	struct rel* next;
	struct rel* prec;
	//int ID;
} relation;

typedef struct rb_node{
	//Order relation is greatest score first and alphanumerical in cases for which two nodes have the same score.
	struct rb_node* p;
	struct rb_node* left;
	struct rb_node* right;
	int score;	
	entity* ent;
	char colour;	
} rb_node;

typedef struct rb_tree{
	rb_node* root;
	int size;
} rb_tree;

typedef struct scoreboard{
	rb_tree* ent_tree;
	rb_node* max_score_node;
	struct scoreboard_entry **hash_table;
} scoreboard_t;

typedef struct scoreboard_entry{
	rb_node* node;
	struct scoreboard_entry* prec;
	struct scoreboard_entry* next;
} scoreboard_entry_t;

typedef struct rel_type{
	char* name;
	struct rel_type* prec;
	struct rel_type* next;
	relation** hash_table;
        scoreboard_t* scoreboard;
} relation_type;

typedef struct ent_rel_list{ 
	relation_type* rel_type;
	//Structured as nodes of a BST
	relation* rel;
	/*
	struct ent_rel_list* left;	
	struct ent_rel_list* right;
	struct ent_rel_list* p;
	*/
	struct ent_rel_list* prec;
	struct ent_rel_list* next;
} entity_relation_list;

typedef enum data_type{
	ENTITY,
	REL,
	REL_TYPE,
	REL_COUNT,
	ENT_REL_LIST,
	SCOREBOARD
} data_t;

// FUNCTION PROTOTYPES

// Linked Lists utility functions

// Insertion
void ent_list_head_insert(entity* new_entity, entity** list_head);
void rel_list_head_insert(relation* new_relation, relation** list_head);
void rel_type_list_head_insert(relation_type* new_rel_type, relation_type** list_head);
void ent_rel_list_head_insert(entity_relation_list* new_ent_rel, entity_relation_list** list_head);

// Search
entity* find_entity_in_list(entity* list_head, char* name);
relation* find_relation_in_list(relation* list_head, char* from, char* to);
relation_type* find_relation_type_in_list(relation_type* list_head, char* name);

// Print
void print_entity_list(entity* list_head);
void print_relation_list(relation* list_head);
void print_rel_type_list(relation_type* list_head);
void print_entity_relation_list(entity_relation_list* list_head);

// Hash Tables utility functions

// Insertion
void entity_hash_table_insert(entity**, entity*, unsigned int);
void relation_hash_table_insert(relation**, relation*, unsigned int);

// Creation
entity** get_new_entity_hash_table();
relation** get_new_rel_hash_table();
scoreboard_entry_t** get_new_scoreboard_hash_table();

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

void rel_list_head_insert(relation* new_relation, relation** list_head){
	//Inserts a relation as head of list_head
	if(verbose){
		printf("[DEBUG] ");
		printf("Inserting relation from %s to %s at head of list pointed by %p ", new_relation->from->name , new_relation->to->name, (void*)list_head);
		if(*list_head == NULL){
			printf("(Old head of list was NULL)\n");
		} else {
			printf("(Old head of list was from %s to %s)\n", (*list_head)->from->name, (*list_head)->to->name);
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
		printf("[DEBUG] Inserting (in order) relation type %s at head of list pointed by %p ", new_rel_type->name, (void*) list_head);
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

void ent_rel_list_head_insert(entity_relation_list* new_ent_rel, entity_relation_list** list_head){
	if(verbose){
		printf("[DEBUG] ");
		printf("Inserting relation from %s to %s of type %s to entity relation list at head of list pointed by %p ", new_ent_rel->rel->from->name, new_ent_rel->rel->to->name, new_ent_rel->rel_type->name , (void*)list_head);
		if(*list_head==NULL){
			printf("(Old head of list was NULL)");
		} else {
			printf("(Old head of list was of type %s, going from %s to %s)\n", (*list_head)->rel_type->name, (*list_head)->rel->from->name, (*list_head)->rel->to->name);
		}
	}
	new_ent_rel->next = *list_head;
	if(*list_head!=NULL){
		(*list_head)->prec = new_ent_rel;
	}
	new_ent_rel->prec = NULL;
	*list_head = new_ent_rel;
}

void scoreboard_entry_list_head_insert(scoreboard_entry_t* new_entry, scoreboard_entry_t** list_head){
	if(verbose){
		printf("[DEBUG] ");
		printf("Inserting new scoreboard node for entity %s with score %d in list pointed by %p", new_entry->node->ent->name, new_entry->node->score, (void*)list_head);
		printf("(Old head of list was %s)\n", *list_head == NULL ? "NULL" : (*list_head)->node->ent->name);
	}
	new_entry->next = *list_head;
	if(*list_head!=NULL){
		(*list_head)->prec = new_entry;
	}
	new_entry->prec = NULL;
	*list_head = new_entry;
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
	}
	free(to_remove);
}

void relation_list_delete(relation* to_remove, relation** list_head){
	if(verbose){
		printf("[DEBUG] ");
		printf("Deleting relation from %s to %s in list pointed by %p \n", to_remove->from->name, to_remove->to->name, (void*)list_head);
	}
	if(to_remove->next){
		to_remove->next->prec = to_remove->prec;
	}
	if(to_remove->prec){
		to_remove->prec->next = to_remove->next;
	}
	free(to_remove);
}

void ent_rel_list_head_delete(entity_relation_list** list_head){
	//For the entity to be deleted, it must be manually done through delete_relation
	if(*list_head){
		entity_relation_list* to_delete = *list_head;
		*list_head = (*list_head)->next;
		if(*list_head){
			(*list_head)->prec = NULL;
		}
		free(to_delete);
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

relation* find_relation_in_list(relation* list_head, char* from, char* to){
	while(list_head!=NULL){
		if(strcmp(list_head->from->name, from)==0 && strcmp(list_head->to->name, to) == 0 ){
			if(verbose){
				printf("[DEBUG] ");
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
				printf("[DEBUG] ");
				printf("Found relation type in list\n");
			}
			return list_head;
		}
		list_head = list_head->next;
	}
	return NULL;
}

scoreboard_entry_t* find_scoreboard_entry_in_list(scoreboard_entry_t* list_head, entity* ent){
	while(list_head!=NULL){
		if(list_head->node->ent == ent){ //Pointer comparison, should both point to the same memory area
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

relation* find_relation(relation_type* current_rel_type, char* from, char* to){
	unsigned int position = hash(string_compactor(3, current_rel_type->name, from, to), REL);
	relation* current_relation = find_relation_in_list(current_rel_type->hash_table[position], from, to);
	return current_relation;
}
/*  
int is_relation_monitored(relation_type* current_rel_type, char* from, char* to){
	unsigned int position = hash(string_compactor(3, current_rel_type->name, from, to), REL);
	relation* current_relation = find_relation_in_list(current_relation_type->hash_table[position], from, to);
	return current_relation ? 1 : 0;
}
*/

void print_entity_list(entity* list_head){
	while(list_head!=NULL){
		printf("[PRINT] (%p) ", (void*)list_head);
		printf("Entity Name %s\n", list_head->name);
		list_head = list_head->next;
	}
}

void print_relation_list(relation* list_head){
	while(list_head!=NULL){
		printf("[PRINT] (%p) ", (void*)list_head);
		printf("Relation from %s to %s\n", list_head->from->name, list_head->to->name);
		list_head = ((relation*)list_head)->next;
	}
}

void print_rel_type_list(relation_type* list_head){
	while(list_head!=NULL){
		printf("[PRINT] (%p) ", (void*)list_head);
		printf("Relation Type: %s\n", list_head->name);
		list_head = ((relation_type*)list_head)->next;
	}
}

void print_entity_relation_list(entity_relation_list* list_head){
	while(list_head!=NULL){
		printf("[PRINT] (%p) ", (void*)list_head);
		printf("Relation from %s to %s\n", list_head->rel->from->name, list_head->rel->to->name);
		list_head = ((entity_relation_list*)list_head)->next;
	}

}

void print_scoreboard_entry_list(scoreboard_entry_t* list_head){
	while(list_head!=NULL){
		printf("[PRINT] (%p) ", (void*)list_head);
		printf("Scoreboard entry with score %d and entity %s\n", list_head->node->score, list_head->node->ent->name);
		list_head = list_head->next;
	}
}

void entity_hash_table_insert(entity** hash_table, entity* ent, unsigned int position){
	ent_list_head_insert(ent, &hash_table[position]); 
}

void relation_hash_table_insert(relation** hash_table, relation* rel, unsigned int position){
	rel_list_head_insert(rel, &hash_table[position]);
}

void scoreboard_hash_table_insert(scoreboard_entry_t** hash_table, scoreboard_entry_t* new_entry, unsigned int position){
	scoreboard_entry_list_head_insert(new_entry, &hash_table[position]);
}

entity* find_entity(entity** hash_table, char* name){
	unsigned int position = hash(string_compactor(1, name), ENTITY);
	return find_entity_in_list(hash_table[position], name);
}

scoreboard_entry_t* find_scoreboard_entry(relation_type* current_rel_type, entity* ent){
	unsigned int position = hash(string_compactor(2, ent->name, current_rel_type->name), SCOREBOARD);
	return find_scoreboard_entry_in_list(current_rel_type->scoreboard->hash_table[position], ent);
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

scoreboard_entry_t** get_new_scoreboard_hash_table(){
	scoreboard_entry_t** hash_table_ptr = calloc(SCOREBOARD_HASH_TABLE_SIZE, sizeof(*hash_table_ptr));
	return hash_table_ptr;
}

void print_entity_hash_table(entity** hash_table, size_t hash_table_size){
	int i;
	for(i=0; i<hash_table_size; i++){
		printf("[PRINT] (%p) ", (void*)hash_table);
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
		printf("[PRINT] (%p) ", (void*)hash_table);
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

void print_scoreboard_hash_table(scoreboard_entry_t** hash_table, size_t hash_table_size){
	int i;
	for(i=0; i<hash_table_size; i++){
		printf("[PRINT] (%p) ", (void*)hash_table);
		if(hash_table[i]==NULL){
			if(suppress_NULL){
				continue;
			}
			printf("Position: %d - Content: NULL\n", i);
		} else {
			printf("Position: %d - Non empty cell: printing list...\n", i);
			print_scoreboard_entry_list(hash_table[i]);
		}
	}
}

void print_all_relations(relation_type* global_relation_type_list){
	while(global_relation_type_list != NULL){
		printf("[PRINT] ");
		printf("Printing all relations of type %s\n", global_relation_type_list->name);
		print_relation_hash_table(global_relation_type_list->hash_table, REL_HASH_TABLE_SIZE);
		global_relation_type_list = global_relation_type_list->next;
	}
}

void print_all_scoreboard_entries(relation_type* global_relation_type_list){
	while(global_relation_type_list != NULL){
		printf("[PRINT] ");
		printf("Printing the scoreboard for relation of type %s\n", global_relation_type_list->name);
		printf("Printing the hash table: \n");
		print_scoreboard_hash_table(global_relation_type_list->scoreboard->hash_table, SCOREBOARD_HASH_TABLE_SIZE);
		printf("Printing the tree: \n");
		rb_preorder_tree_walk(global_relation_type_list->scoreboard->ent_tree->root);
		global_relation_type_list = global_relation_type_list->next;
	}

}
int rb_node_compare(rb_node* x, rb_node* y){ //Acts as a > 'greater than' sign
	if(x==NULL && y==NULL){
		return 0;
	}
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

void rb_insert_fixup(rb_tree* tree, rb_node* z){
	rb_node *x, *y;
	if(z == tree->root){
		tree->root->colour = 'b';
	}else{
		x = z->p;
		if(x->colour == 'r'){
			if(x == x->p->left){
				y = x->p->right;
				if(y!= NULL && y->colour == 'r'){
					x->colour = 'b';
					y->colour = 'b';
					x->p->colour = 'r';
					rb_insert_fixup(tree, x->p);
				} else {
					if (z == x->right){
						z = x;
						rb_left_rotate(tree, z);
						x = z->p;
					}
					x->colour = 'b';
					x->p->colour = 'r';
					rb_right_rotate(tree, x->p);
				}
			} else {
				y = x->p->left;
				if(y != NULL && y->colour == 'r'){
					x->colour = 'b';
					y->colour = 'b';
					x->p->colour = 'r';
					rb_insert_fixup(tree, x->p);
				} else {
					if (z == x->left){
						z = x;
						rb_right_rotate(tree, z);
						x = z->p;
					}
					x->colour = 'b';
					x->p->colour = 'r';
					rb_left_rotate(tree, x->p);
				}
			}
		}
	}
}

void rb_delete_fixup(rb_tree* tree, rb_node* x, rb_node* parent,  char side){
	rb_node* w;
	if(x!=NULL && (x->colour == 'r' || x->p == NULL)){
		x->colour = 'b';
	} else if(side == 'l'){
		w = parent->right;
		if(w->colour == 'r'){
			w->colour = 'b';
			parent->colour = 'r';
			rb_left_rotate(tree, parent);
		}
		if((w->left == NULL || w->left->colour == 'b') && (w->right == NULL || w->right->colour == 'b')){
			w->colour = 'r';
			if(parent->p==NULL){
				rb_delete_fixup(tree, parent, NULL, 'l');
			} else if(parent == parent->p->left){
				rb_delete_fixup(tree, parent, parent->p, 'l');
			} else {
				rb_delete_fixup(tree, parent, parent->p, 'r');
			}
		} else {
			if(w->right == NULL || w->right->colour == 'b'){
				w->left->colour = 'b';
				w->colour = 'r';
				rb_right_rotate(tree, w);
				w = parent->right;
			}
			w->colour = parent->colour;
			parent->colour = 'b';
			w->right->colour = 'b';
			rb_left_rotate(tree, parent);
		}
	} else if(side == 'r') {
		w = parent->left;
		if(w->colour == 'r'){
			w->colour = 'b';
			parent->colour = 'r';
			rb_right_rotate(tree, parent);
		}
		if((w->right == NULL || w->right->colour == 'b') && (w->left == NULL || w->left->colour == 'b')){
			w->colour = 'r';
			if(parent->p == NULL){
				rb_delete_fixup(tree, parent, NULL, 'l');
			} else if(parent == parent->p->left){
				rb_delete_fixup(tree, parent, parent->p, 'l');
			} else {
				rb_delete_fixup(tree, parent, parent->p, 'r');
			}
		} else {
			if(w->left == NULL || w->left->colour == 'b'){
				w->right->colour = 'b';
				w->colour = 'r';
				rb_left_rotate(tree, w);
				w = parent->left;
			}
			w->colour = parent->colour;
			parent->colour = 'b';
			w->left->colour = 'b';
			rb_right_rotate(tree, parent);
		}
	} else {
		printf("Invalid Side\n");
		return;
	}
}

int rb_delete(rb_tree* tree, rb_node* z){
	//Deletes node and returns its key
	rb_node* y = NULL;
	rb_node* x = NULL;
	char side;
	int to_return = z->score;
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
		tree->root = x;
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
			tree->root = y;
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
		rb_delete_fixup(tree, x, parent, side);
	}
	free(z);
	(tree->size)--;
	return to_return;
}

void rb_insert(rb_tree* tree, rb_node* z){
	rb_node* y = NULL;
	rb_node* x = tree->root;
	while(x!=NULL){
		y = x;
		//if((*z)->score < x->score){
		if(!rb_node_compare(z, x)){
			x = x->left;
		} else {
			x = x->right;
		}
	}	
	z->p = y;
	if(y == NULL){
		tree->root = z;
	//}else if((*z)->score < y->score){
	} else if(!rb_node_compare(z, y)) {
		y->left = z;
	} else {
		y->right = z;
	}
	z->left = NULL;
	z->right = NULL;
	z->colour = 'r';
	rb_insert_fixup(tree, z);
	(tree->size)++;
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
		for(j=0; j<strlen(currString); j++){
			currSum = currSum*33 + (int)currString[i];
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
		case SCOREBOARD:
			return key%SCOREBOARD_HASH_TABLE_SIZE;
		default:
			return 0;
	}
}

entity* add_new_entity(entity** hash_table, char* name){
	entity* new_entity = malloc(sizeof(entity));
	new_entity->name = malloc(strlen(name)+1);
	new_entity->rel_list = NULL;
	strcpy(new_entity->name, name);
	unsigned int position = hash(string_compactor(1,name), ENTITY);
	if(verbose){
		printf("[DEBUG] ");
		printf("Adding new entity %s in position %d of hash table\n", name, position);
	}
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
	new_rel_type->scoreboard = malloc(sizeof(scoreboard_t));
	rel_type_list_ordered_insert(new_rel_type, relation_type_list);
	initialize_new_rel_type(new_rel_type);
	return new_rel_type;
}

entity_relation_list* add_relation_to_entities(entity* entity1, entity* entity2, relation_type* current_rel_type, relation* new_relation){
	entity_relation_list* new_entity_relation = malloc(sizeof(entity_relation_list));
	new_entity_relation->rel_type = current_rel_type;
	new_entity_relation->rel = new_relation;
	ent_rel_list_head_insert(new_entity_relation, &(entity1->rel_list));		
	if(strcmp(entity1->name, entity2->name)!=0){
		ent_rel_list_head_insert(new_entity_relation, &(entity2->rel_list));
	}
	return new_entity_relation;
}

scoreboard_entry_t* add_entry_to_scoreboard(entity* ent, relation_type* current_rel_type, int score){
	rb_node* new_node = malloc(sizeof(rb_node));
	new_node->score = score;
	new_node->ent = ent;
	unsigned int position = hash(string_compactor(2, ent->name, current_rel_type->name), SCOREBOARD);
	scoreboard_entry_t *new_scoreboard_entry = malloc(sizeof(scoreboard_entry_t));
	new_scoreboard_entry->node = new_node;
	scoreboard_hash_table_insert(current_rel_type->scoreboard->hash_table, new_scoreboard_entry, position);	
	if(current_rel_type->scoreboard->ent_tree->root == NULL || rb_node_compare(new_node, current_rel_type->scoreboard->max_score_node)>0){
		current_rel_type->scoreboard->max_score_node = new_node;
	}
	rb_insert(current_rel_type->scoreboard->ent_tree, new_node);
	return new_scoreboard_entry;
}

void update_scoreboard_entry(int score_difference, scoreboard_entry_t* current_entry, relation_type* current_rel_type){
	//scoreboard_entry_t* current_entry = find_scoreboard_entry(current_rel_type, ent);
	rb_node* to_delete = current_entry->node;
	rb_node* new_node = (rb_node*)malloc(sizeof(rb_node));
	new_node->score = current_entry->node->score + score_difference;
	new_node->ent = current_entry->node->ent;
	if(strcmp(new_node->ent->name, current_rel_type->scoreboard->max_score_node->ent->name) && rb_node_compare(new_node, current_rel_type->scoreboard->max_score_node)){
		//If the max node is lesser than the adjourned one, change the max node. Also check if they are different nodes
		current_rel_type->scoreboard->max_score_node = new_node;
	}
	rb_delete(current_rel_type->scoreboard->ent_tree, to_delete);
	current_entry->node = new_node;
	rb_insert(current_rel_type->scoreboard->ent_tree, new_node);
}

void delete_scoreboard_entry(scoreboard_entry_t* current_entry, relation_type* current_rel_type){
	rb_node* to_delete = current_entry->node;
	rb_delete(current_rel_type->scoreboard->ent_tree, to_delete);
}

void print_all_scoreboards(relation_type* global_relation_type_list){
	printf("[PRINT] ");
	printf("Printing all scoreboards: \n");
	while(global_relation_type_list){
		int i = 0;
		rb_node* current_node = rb_tree_maximum(global_relation_type_list->scoreboard->ent_tree->root);
		printf("[PRINT] ");
		printf("Printing scoreboard for relationship %s\n", global_relation_type_list->name);
		while(current_node){
			printf("(%d) | %s [%d]\n", i, current_node->ent->name, current_node->score);
			current_node = rb_tree_predecessor(current_node);
			i++;
		}
		global_relation_type_list = global_relation_type_list->next;
	}
}

void delete_relation(relation_type* current_rel_type, relation* to_delete){
	unsigned int position = hash(string_compactor(3, to_delete->from->name, to_delete->to->name, current_rel_type->name), REL);
	relation_list_delete(to_delete, &(current_rel_type->hash_table[position]));
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
			free(nav->hash_table); //Assuming that the hash_table is already empty
			free(nav->scoreboard->ent_tree); //Assuming that the tree is empty
			free(nav->scoreboard->hash_table); //Also assuming that this hash table is empty
			free(nav->scoreboard);
			free(nav);
			return;
		}
		nav = nav->next;
	}
}

void clear_all_entity_relations(entity_relation_list** rel_list){
	entity_relation_list* nav = *rel_list;
	while(nav){
		delete_relation(nav->rel_type, nav->rel);
		nav = nav->next;
		ent_rel_list_head_delete(rel_list);
	}
}

void initialize_global_structure(entity*** hash_table, relation_type** relation_type_list){
	//Initializes every data structure necessary for the program
	*hash_table = get_new_entity_hash_table();
	*relation_type_list = NULL;
}

void cleanup(entity*** hash_table, relation_type** relation_type_list){
	//Clears every data structure at program end	
}

void initialize_rb_tree(rb_tree* tree){
	tree->root = NULL;
	tree->size = 0;
}

void initialize_new_rel_type(relation_type* new_rel_type){
	new_rel_type->hash_table = get_new_rel_hash_table();
	new_rel_type->scoreboard->hash_table = get_new_scoreboard_hash_table();
	new_rel_type->scoreboard->max_score_node = NULL;
	new_rel_type->scoreboard->ent_tree = malloc(sizeof(rb_tree));
	initialize_rb_tree(new_rel_type->scoreboard->ent_tree);	
}

void addent(entity** global_entity_hash_table, char* name){
	//Expected complexity O(1)
	if(verbose){
		printf("[DEBUG] ");
		printf("addent: entity %s\n", name);
	}
	if(!find_entity(global_entity_hash_table, name)){
		//If entity does not already exist then add it
		add_new_entity(global_entity_hash_table, name);
	}
}

void delent(entity** global_entity_hash_table, relation_type* global_relation_type_list, char* name){
	//Check if entity actually exists and tries to fetch it
	entity* current_entity = find_entity(global_entity_hash_table, name);
	if(current_entity){
		clear_all_entity_relations(&(current_entity->rel_list));
		while(global_relation_type_list){
			scoreboard_entry_t* current_scoreboard_entry = find_scoreboard_entry(global_relation_type_list, current_entity);
			if(current_scoreboard_entry){
				delete_scoreboard_entry(current_scoreboard_entry, global_relation_type_list);	
			}
		}
	}
}

void addrel(entity** global_entity_hash_table, relation_type** global_relation_type_list, char* from, char* to, char* name){
	//Expected complexity O(log(e)) 
	if(verbose){
		printf("[DEBUG] ");
		printf("addrel: from %s to %s. Type: %s\n", from, to, name);
	}
	//Find if both entities that are involved exist
	entity* ent_from = find_entity(global_entity_hash_table, from);
	entity* ent_to = find_entity(global_entity_hash_table, to);
	if(ent_from && ent_to){
		//If both entities actually exist then attempt an insertion of a new relation between ent_from and ent_to
		relation_type* current_rel_type = find_relation_type_in_list(*global_relation_type_list, name);
		if(!current_rel_type){
			//If the current relation type does not exist then create it
			current_rel_type = add_new_relation_type(global_relation_type_list, name);	
			//And create a new entry for the scoreboard
			add_entry_to_scoreboard(ent_to, current_rel_type, 1);
		} else {
			//If the current relation type does exist...
			if(find_relation(current_rel_type, from, to)){
				//...check if the relation already exists...
				return;
			}
			//...if not, then check if there's an existing scoreboard entry for the receiving entity
			scoreboard_entry_t* current_scoreboard_entry = find_scoreboard_entry(current_rel_type, ent_to);
			if(current_scoreboard_entry){
				//If the scoreboard entry exists, then update it, adding 1 to its score
				update_scoreboard_entry(1, current_scoreboard_entry, current_rel_type);
			} else {
				//Otherwise create a new scoreboard entry with score 1
				add_entry_to_scoreboard(ent_to, current_rel_type, 1);
			}
		}
		//Create the new relation object and add it to the hash table of relations
		relation* new_relation = add_new_relation(ent_from, ent_to, current_rel_type);
		//Also insert a reference to the newly created relation in the entity relation list
		add_relation_to_entities(ent_from, ent_to, current_rel_type, new_relation);
	}
}

void delrel(entity** global_entity_hash_table, relation_type** global_relation_type_list, char* from, char* to, char* name){
	//Check if relation type exists
	relation_type* current_rel_type = find_relation_type_in_list(*global_relation_type_list, name);
	if(current_rel_type){
		//If so, find both entities involved
		entity* ent_from = find_entity(global_entity_hash_table, from);
		entity* ent_to = find_entity(global_entity_hash_table, to);
		if(ent_from && ent_to){
			//If both entities exist, then check if the relation that we need to delete exists
			relation* current_relation = find_relation(current_rel_type, from, to);
			if(!current_relation){
				//If no such relation exists, then do nothing
				return;
			}
			//Otherwise we need to actually delete the relation: update the scoreboard, removing 1 from the score
			scoreboard_entry_t* current_scoreboard_entry = find_scoreboard_entry(current_rel_type, ent_to);
			update_scoreboard_entry(-1, current_scoreboard_entry, current_rel_type);
			//Delete the relation object
			delete_relation(current_rel_type, current_relation);
			//TODO: Delete the relation reference from the entities (?)
			if(current_rel_type->scoreboard->ent_tree->size == 0){
				relation_type_list_delete(global_relation_type_list, current_rel_type);
			}
		}
	}
}

void report(relation_type* global_relation_type_list){
	relation_type* nav = global_relation_type_list;
	char* output_string = calloc(OUTPUT_BUFFER_SIZE, sizeof(char));
	int first_line = 1;
	while(nav){
		char* current_line = calloc(LINE_BUFFER_SIZE, sizeof(char));
		if(!first_line){
			//Adding space after semicolon if needed
			sprintf(current_line, " \"%s\"", nav->name);
		} else {
			sprintf(current_line, "\"%s\"", nav->name);
		}
		rb_node* max_node = rb_tree_maximum(nav->scoreboard->ent_tree->root);
		rb_modified_inorder_tree_walk(nav->scoreboard->ent_tree->root, current_line, max_node->score);
		char points[12];
		sprintf(points, " %d;", max_node->score);
		strcat(current_line, points);
		strcat(output_string, current_line);
		nav = nav->next;
		if(first_line){
			first_line = 0;
		}
		free(current_line);
	}
	if(strlen(output_string)){
		puts(output_string);
	} else {
		puts("none");
	}
	free(output_string);
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
			if(strcmp(inputBuffer, "end")==0){
				return 1;
			} 
			command = strtok(inputBuffer, separators);
			if(strcmp(command, "addent")==0){
				arg1 = strtok(NULL, separators);	
				addent(global_entity_hash_table, arg1);
			} else if(strcmp(command, "addrel")==0){
				arg1 = strtok(NULL, separators);
				arg2 = strtok(NULL, separators);
				arg3 = strtok(NULL, separators);
				addrel(global_entity_hash_table, &global_relation_type_list, arg1, arg2, arg3);
			} else if(strcmp(command, "delent")==0){
				arg1 = strtok(NULL, separators);
				delent(global_entity_hash_table, global_relation_type_list, arg1);
			} else if(strcmp(command, "delrel")==0){
				arg1 = strtok(NULL, separators);
				arg2 = strtok(NULL, separators);
				arg3 = strtok(NULL, separators);
				delrel(global_entity_hash_table, &global_relation_type_list, arg1, arg2, arg3);
			} else if(strcmp(command, "report")==0){
				report(global_relation_type_list);
			}
			if(verbose){
				print_all_scoreboards(global_relation_type_list);
			}
		} else {
			if(verbose){
				printf("Failed to read line: terminating program\n");
			}
		}
	}
}

