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
#include <assert.h>

// VARIOUS CONSTANTS

#define REL_TYPE_ARRAY_SIZE 10
#define ENTITY_HASH_TABLE_SIZE 500
#define MAX_HASH_VALUE UINT_MAX

#define INPUT_BUFFER_SIZE 400
#define OUTPUT_BUFFER_SIZE 1
#define LINE_BUFFER_SIZE 1

#define ADDENT_CACHE_ENABLED 1
#define CACHE_ENABLED 1

//Uncomment the next line to show verbose log messages
//#define VERBOSE 

//Set this constant to 0 to skip over NULL cells when printing hash tables
#define SUPPRESS_NULL 1

//CHARACTER ASCII VALUES

#define DQUOTES 34
#define SPACE 32
#define RETURN_CARRIAGE 12
#define SEMICOLON 59

//TODO: Check for failed mallocs on all functions
//TODO: Rehashing when load factor is greater than 0.75

// STRUCTURES DECLARATIONS

struct ent;                // Entity structure.
struct rel;                // Relation structure: contains pointer information about the two entities concerned.
struct rel_type;           // Relation Type structure: contains the type of relation.
struct scoreboard_entry;            // Node of a Red-Black Tree: used for scoreboard purposes. Each node contains the score of an entity
struct rel_type_reference;

typedef struct ent{
	char* name;
	struct scoreboard_entry* scoreboard_entry_list;
	struct ent* next;
	struct rel* relation_root;
} entity;

typedef struct rel{
	entity* from;
	unsigned int rel_type_present;
	struct rel *p, *left, *right;
} relation;

typedef struct scoreboard_entry{
	unsigned short int score;	
	char colour;	
	entity* ent;
	struct rel_type* rel_type;
	struct scoreboard_entry* p;
	struct scoreboard_entry* left;
	struct scoreboard_entry* right;
	struct scoreboard_entry* next;
} scoreboard_entry;

typedef struct rel_type{
	char* name;
	scoreboard_entry* root;
	struct rel_type_reference* reference;
} relation_type;

typedef struct rel_type_reference{
	struct rel_type* rel_type;
} rel_type_reference;

typedef enum data_type{
	ENTITY,
	REL,
	REL_TYPE,
} data_t;

// CACHE VARIABLES

char* current_line; 

entity* cached_entity_A = NULL;
entity* cached_entity_B = NULL;
relation_type* cached_relation_type = NULL;

// FUNCTION PROTOTYPES

// ABSTRACTION LEVEL 0: Functions concerning basic data structure management

//List Utilities
void ent_list_head_insert(entity* new_entity, entity** list_head);
void scoreboard_entry_list_head_insert(scoreboard_entry* new_node, scoreboard_entry** list_head);

void entity_list_delete(entity* to_remove, entity** list_head);
void scoreboard_entry_list_delete(scoreboard_entry* to_remove, scoreboard_entry** list_head);

void print_entity_list(entity* list_head);
void print_entity_hash_table(entity** hash_table, size_t hash_table_size);

entity* find_entity_in_list(char* name, entity* list_head);
scoreboard_entry* find_scoreboard_entry_in_list(relation_type* current_rel_type, scoreboard_entry* list_head);

//Relation Tree Utilities
void relation_tree_precision_insert(relation** root, relation* z, relation* insertion_point);
void relation_tree_insert(relation** root, relation* z, unsigned int index);

void relation_tree_delete(relation** root, relation* z);

relation* relation_tree_search(relation* root, entity* from, relation** insertion_point);

relation* relation_tree_minimum(relation* z);
relation* relation_tree_successor(relation* z);

void relation_preorder_tree_walk(relation* node, rel_type_reference** relation_type_reference_list);

int relation_node_compare(relation* x, relation* y);
int relation_node_string_compare(relation* x, entity* from);

//Scoreboard Tree Utilities

int scoreboard_entry_compare(scoreboard_entry* x, scoreboard_entry* y);
void scoreboard_left_rotate(scoreboard_entry** root, scoreboard_entry* x);
void scoreboard_right_rotate(scoreboard_entry** root, scoreboard_entry* x);
scoreboard_entry* scoreboard_tree_minimum(scoreboard_entry* z);
scoreboard_entry* scoreboard_tree_maximum(scoreboard_entry* z);
scoreboard_entry* scoreboard_tree_successor(scoreboard_entry* z);
scoreboard_entry* scoreboard_tree_predecessor(scoreboard_entry* z);
void scoreboard_inorder_tree_walk(scoreboard_entry* node);
void scoreboard_preorder_tree_walk(scoreboard_entry* node);
void scoreboard_report_inorder_tree_walk(scoreboard_entry* node, char* ent_string, int score_to_search); //Change this name

void scoreboard_insert(scoreboard_entry** root, scoreboard_entry* z);
void scoreboard_delete(scoreboard_entry** root, scoreboard_entry* z);
void scoreboard_insert_fixup(scoreboard_entry** root, scoreboard_entry* z);
void scoreboard_delete_fixup(scoreboard_entry** root, scoreboard_entry* x, scoreboard_entry* parent, char side);

//Hashing Utilities

unsigned int hash_jdb2(int num, ...);

//Hash Table Utilities

void entity_hash_table_insert(entity* ent, unsigned int position, entity** hash_table);
entity** get_new_entity_hash_table();

//Relation Type Array Utilities

void fix_relation_type_references(relation_type** global_relation_type_list, rel_type_reference** relation_type_reference_list);

//ABSTRACTION LEVEL 1: Functions concerning problem-related data structure queries

//Insertion

relation_type* add_new_relation_type(char* name, relation_type** relation_type_list, rel_type_reference** relation_type_reference_list);
scoreboard_entry* add_scoreboard_entry(entity* ent, relation_type* current_rel_type, int score);
void update_scoreboard_entry(scoreboard_entry* current_entry, relation_type* current_rel_type, int score_difference);

//Search

relation_type* find_relation_type(char* name, relation_type** rel_type_array);
entity* find_entity(char* name, entity** hash_table);
relation* find_relation(entity* ent_from, entity* ent_to, relation** insertion_point);
scoreboard_entry* find_scoreboard_entry(relation_type* current_rel_type, entity* ent);

//Deletion

void delete_scoreboard_entry(scoreboard_entry* current_entry, relation_type* current_rel_type);
void delete_entity(entity* to_delete, entity** global_entity_hash_table);
void delete_relation(relation* to_delete, entity* ent_to, unsigned int index);

//Initialization

void initialize_global_structure(entity*** hash_table, relation_type*** relation_type_list, rel_type_reference*** relation_type_reference);
void initialize_new_rel_type(relation_type* new_rel_type);

//Relation Type Management

int has_matching_rel_type(relation* current_relation, unsigned int index);
void delete_empty_rel_type(relation_type** relation_type_list);
int is_rel_type_array_empty(relation_type** global_relation_type_list);

//Relation Management

void add_rel_type_to_relation(relation* node, unsigned int rel_type_index);
void remove_rel_type_in_relation(relation* to_delete, unsigned int index);

//ABSTRACTION LEVEL 2: Functions of which the implementation has been requested in the problem statement

void addent(char* name, entity** global_entity_hash_table, relation_type** global_relation_type_list);
void addrel(char* from, char* to, char* name, entity** global_entity_hash_table, relation_type** global_relation_type_list, rel_type_reference** relation_type_reference_list);
void delrel(char* from, char* to, char* name, entity** global_entity_hash_table, relation_type** global_relation_type_list, rel_type_reference** relation_type_reference_list);
void delent(char* name, entity** global_entity_hash_table, relation_type** global_relation_Type_list, rel_type_reference** relation_type_reference_list);
void report(entity** global_entity_hash_table, relation_type** global_relation_type_list);

void ent_list_head_insert(entity* new_entity, entity** list_head){
	//Inserts a relation as head of list_head
	
	#ifdef VERBOSE
		printf("[DEBUG] ");
		printf("Inserting entity %s at head of list pointed by %p ", new_entity->name, (void*)list_head);
		printf("(Old head of list was %s)\n", *list_head == NULL ? "NULL" : (*list_head)->name);
	#endif

	new_entity->next = *list_head;
	*list_head = new_entity;
}

void scoreboard_entry_list_head_insert(scoreboard_entry* new_node, scoreboard_entry** list_head){
	//Inserts an scoreboard_entry as head of list_head
	
	#ifdef VERBOSE
		printf("[DEBUG] ");
		printf("Inserting scoreboard entry node for entity %s  at head of list pointed by %p ", new_node->ent->name, (void*)list_head);
		printf("(Old head of list was %s)\n", *list_head == NULL ? "NULL" : (*list_head)->ent->name);
	#endif

	new_node->next = *list_head;
	*list_head = new_node;
}

void entity_list_delete(entity* to_remove, entity** list_head){
	//Deletes to_remove entity from list
	
	#ifdef VERBOSE
		printf("[DEBUG] ");
		printf("Deleting entity %s in list pointed by %p \n", to_remove->name, (void*)list_head);
	#endif

	entity* nav = *list_head;
	entity* follow = NULL;
	while(nav){
		if(nav == to_remove){
			break;
		}
		follow = nav;
		nav = nav->next;
	}	
	if(follow){
		follow->next = nav->next;
	} else {
		*list_head = nav->next;
	}
	free(to_remove->name);
	free(to_remove);
}

void scoreboard_entry_list_delete(scoreboard_entry* to_remove, scoreboard_entry** list_head){
	//Deletes to_remove from list
	scoreboard_entry* nav = *list_head;
	scoreboard_entry* follow = NULL;
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

entity* find_entity_in_list(char* name, entity* list_head){
	//Looks up a list to find entity. Used to search for a name in the hash table chains
	while(list_head!=NULL){
		if(strcmp(list_head->name, name)==0){
			#ifdef VERBOSE
				printf("[DEBUG] ");
				printf("Found entity %s in list\n", name);
			#endif
			return list_head;
		}
		list_head = list_head->next;
	}
	return NULL;
}

relation_type* find_relation_type(char* name, relation_type** rel_type_array){
	//Looks up the relation type list to find the relation_type instance
	int i = 0;
	for(i=0; i<REL_TYPE_ARRAY_SIZE; i++){
		if(rel_type_array[i] && strcmp(rel_type_array[i]->name, name)==0){
			return rel_type_array[i];
		}
	}
	return NULL;
}

scoreboard_entry* find_scoreboard_entry_in_list(relation_type* current_rel_type, scoreboard_entry* list_head){
	//Searches for a scoreboard_entry with matching relation type inside a list
	while(list_head!=NULL){
		if(list_head->rel_type == current_rel_type){ 
			#ifdef VERBOSE
				printf("[DEBUG] ");
				printf("Found scoreboard entry in list\n");
			#endif
			return list_head;
		}
		list_head = list_head->next;
	}
	return NULL;
}	

int has_matching_rel_type(relation* current_relation, unsigned int index){
	//Checks if current_relation is valid for a given relation type, indicated by its index value
	return current_relation->rel_type_present & (1 << index);
}

relation* find_relation(entity* ent_from, entity* ent_to, relation** insertion_point){
	//Searches for a relation between ent_from and ent_to.
	//Also passes the insertion_point to relation_tree_search
	relation* current_relation = NULL;
	if(ent_to->relation_root){
		current_relation = relation_tree_search(ent_to->relation_root, ent_from, insertion_point);	
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

void entity_hash_table_insert(entity* ent, unsigned int position, entity** hash_table) {
	//Inserts ent in the hash_table at the given position, which should be provided by a precedent call to a hash function
	ent_list_head_insert(ent, &hash_table[position]); 
}

entity* find_entity(char* name, entity** hash_table){
	//Looks up entity in the hash table
	unsigned int position = hash_jdb2(1, name);
	return find_entity_in_list(name, hash_table[position]);
}
  
scoreboard_entry* find_scoreboard_entry(relation_type* current_rel_type, entity* ent){
	//Searches for a scoreboard_entry with matching current_rel_type for the entity provided
	return find_scoreboard_entry_in_list(current_rel_type, ent->scoreboard_entry_list);
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
			#ifdef SUPPRESS_NULL
				continue;
			#else
				printf("[PRINT] (%p) ", (void*)hash_table);
				printf("Position: %d - Content: NULL\n", i);
			#endif
		} else {
			printf("[PRINT] (%p) ", (void*)hash_table);
			printf("Position: %d - Non empty cell: printing list...\n", i);
			print_entity_list(hash_table[i]);
		}
	}
}

int relation_node_compare(relation* x, relation* y){
	//Acts as a comparation callback between two relation nodes in a BST
	//The rules to the ordering are:
	//1. A NULL node should always be lesser than any other node, except another NULL node. In that case, they are considered equal
	//2. If both nodes are non-NULL, then the comparison is given by alphanumerical ordering.
	if(!x && !y){
		return 0;
	} else if (!x){
		return -1;
	} else if (!y){
		return 1;
	}
	int comparison = strcmp(x->from->name, y->from->name);
	if(comparison > 0){ //If the name of x's receiver comes after y's receiver then x>y
		return 1;
	} else if (comparison < 0){ //If the name of y's receiver comes after then x<y
		return -1;
	} else {
		return 0;
	}
}

int relation_node_string_compare(relation* x, entity* from){
	//Same as relation_node_compare, but compares between an existing relation node and an entity (Which should be the content of another node)
	//This is the comparison used in tree insertion, to decide where we should insert a newly node that has yet to be created
	if(x==NULL){
		return -1;
	}
	int comparison = strcmp(x->from->name, from->name);
	if(comparison > 0){
		return 1;
	} else if (comparison < 0){
		return -1;
	} else {
		return 0;
	}
}

relation* relation_tree_search(relation* root, entity* from, relation** insertion_point){
	//Searches for a entity containing the entity "from" in the tree with root "root"
	//Side effect: if the relation is not found, then insertion_point will be set to the node to which we will attach the relation if we need to insert it
	//insertion_point may be set to NULL if this is not required
	int comparison = relation_node_string_compare(root, from);
	if(comparison==0){
		if(insertion_point){
			*insertion_point = root;
		}
		return root;
	}
	if(comparison > 0){
		if(root->left == NULL){
			if(insertion_point){
				*insertion_point = root;	
			}
			return NULL;
		} else{
			return relation_tree_search(root->left, from, insertion_point);
		}
	} else {
		if(root->right == NULL){
			if(insertion_point){
				*insertion_point = root;	
			}
			return NULL;
		} else{
			return relation_tree_search(root->right, from, insertion_point);
		}
	}
}

relation* relation_tree_minimum(relation* z){
	//Returns the minimum element in the relation tree
	while(z->left){
		z = z->left;
	}
	return z;
}

relation* relation_tree_successor(relation* z){
	//Returns the successor element in the relation tree
	//The successor element is defined as the minimum between all the elements that are greater than z
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

void add_rel_type_to_relation(relation* node, unsigned int index){
	//Makes the relation valid for a new relation type, indicated by its index
	node->rel_type_present += 1 << index;
}
 
void relation_tree_insert(relation** root, relation* z, unsigned int index){
	//Inserts a new relation node in the tree and makes it valid for the relation type, indicated by its index
	//The algorithm is adapted from the CLRS book "Introduction to Algorithms"
	relation* y = NULL;
	relation* x = *root;
	while(x){
		y = x;
		if(relation_node_compare(z, x)<0){
			x = x->left;
		} else if (relation_node_compare(z, x)==0){
			add_rel_type_to_relation(z, index);
			return;
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
	//Deletes the node z from the relation tree
	//The algorithm is adapted from the CLRS book "Introduction to Algorithms"
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
		z->rel_type_present = y->rel_type_present;
	}
	free(y);
}

void relation_preorder_tree_walk(relation* node, rel_type_reference** relation_type_reference_list){
	if(node){
		printf("[PRINT] ");
		printf("Printing relation in tree from %s ", node->from->name);
		printf("[ ");
		for(int i = 0; i < REL_TYPE_ARRAY_SIZE; i++){
			if(has_matching_rel_type(node, i)){
				printf("%s ",relation_type_reference_list[i]->rel_type->name);
			}
		}
		printf(" ]\n");
		relation_preorder_tree_walk(node->left, relation_type_reference_list);
		relation_preorder_tree_walk(node->right, relation_type_reference_list);	
	} else {
		printf("[PRINT] ");
		printf("Printing NULL node\n");
	}
}

int scoreboard_entry_compare(scoreboard_entry* x, scoreboard_entry* y){
	//Acts as a comparation callback between two scoreboard entries in an Red-Black Tree
	//The rules to the ordering are:
	//1. A NULL node should always be lesser than any other node. No two NULL nodes shall be compared
	//2. If both nodes are non-NULL, then the comparison is given by the score: the node with the greater score is considered the greater node
	//3. If both nodes have the same score, then the comparison is given by the entity to which they refer, in inverse alphanumerical order.
	//As it is impossible to have two nodes with the same entity name, this completely determines the ordering
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

void scoreboard_left_rotate(scoreboard_entry** root, scoreboard_entry* x){
	//Left-Rotate for Red-Black Tree balancing
	//The algorithm is adapted from the CLRS book "Introduction to Algorithms"
	scoreboard_entry* y = x->right;
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

void scoreboard_right_rotate(scoreboard_entry** root, scoreboard_entry* x){ 
	//Right-Rotate for Red-Black Tree balancing
	//The algorithm is adapted from the CLRS book "Introduction to Algorithms"
	scoreboard_entry* y = x->left;
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

scoreboard_entry* scoreboard_tree_minimum(scoreboard_entry* z){
	//Returns the minimal element of a scoreboard tree
	while(z->left != NULL){
		z = z->left;
	}
	return z;
}

scoreboard_entry* scoreboard_tree_maximum(scoreboard_entry* z){
	//Returns the maximal element of a scoreboard tree
	while(z->right != NULL){
		z = z->right;
	}
	return z;
}

scoreboard_entry* scoreboard_tree_successor(scoreboard_entry* z){
	//Returns the element that is successor to z in the tree
	if(z->right != NULL){
		return scoreboard_tree_minimum(z->right);
	}
	scoreboard_entry* y = z->p;
	while(y!=NULL && z == y->right){
		z = y;
		y = y->p;
	}
	return y;
}

scoreboard_entry* scoreboard_tree_predecessor(scoreboard_entry* z){
	//Returns the element that is predecessor to z in the tree
	if(z->left != NULL){
		return scoreboard_tree_maximum(z->left);
	}
	scoreboard_entry* y = z->p;
	while(y!=NULL && z == y->left){
		z = y;
		y = y->p;
	}
	return y;
}

void scoreboard_insert_fixup(scoreboard_entry** root, scoreboard_entry* z){
	//Red-Black Tree fixup algorithm to maintain RB-Tree properties and balancing
	//The algorithm is adapted from the CLRS book "Introduction to Algorithms"
	scoreboard_entry *x, *y;
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
					scoreboard_insert_fixup(root, x->p);
				} else {
					if (z == x->right){
						z = x;
						scoreboard_left_rotate(root, z);
						x = z->p;
					}
					x->colour = 'b';
					x->p->colour = 'r';
					scoreboard_right_rotate(root, x->p);
				}
			} else {
				y = x->p->left;
				if(y != NULL && y->colour == 'r'){
					x->colour = 'b';
					y->colour = 'b';
					x->p->colour = 'r';
					scoreboard_insert_fixup(root, x->p);
				} else {
					if (z == x->left){
						z = x;
						scoreboard_right_rotate(root, z);
						x = z->p;
					}
					x->colour = 'b';
					x->p->colour = 'r';
					scoreboard_left_rotate(root, x->p);
				}
			}
		}
	}
}

void scoreboard_delete_fixup(scoreboard_entry** root, scoreboard_entry* x, scoreboard_entry* parent,  char side){
	//Red-Black Tree fixup algorithm to maintain RB-Tree properties and balancing
	//The algorithm is adapted from the CLRS book "Introduction to Algorithms"
	scoreboard_entry* w;
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
			scoreboard_left_rotate(root, parent);
			w = parent->right;
		}
		if((w->left == NULL || w->left->colour == 'b') && (w->right == NULL || w->right->colour == 'b')){
			w->colour = 'r';
			if(parent->p==NULL){
				//Side doesn't matter here, execution will stop after the first if because x->p = NULL
				scoreboard_delete_fixup(root, parent, NULL, 'l');
			} else if(parent == parent->p->left){
				scoreboard_delete_fixup(root, parent, parent->p, 'l');
			} else {
				scoreboard_delete_fixup(root, parent, parent->p, 'r');
			}
		} else {
			if(w->right == NULL || w->right->colour == 'b'){
				w->left->colour = 'b';
				w->colour = 'r';
				scoreboard_right_rotate(root, w);
				w = parent->right;
			}
			w->colour = parent->colour;
			parent->colour = 'b';
			w->right->colour = 'b';
			scoreboard_left_rotate(root, parent);
		}
	} else if(side == 'r') {
		w = parent->left;
		if(w->colour == 'r'){
			w->colour = 'b';
			parent->colour = 'r';
			scoreboard_right_rotate(root, parent);
			w = parent->left;
		}
		if((w->right == NULL || w->right->colour == 'b') && (w->left == NULL || w->left->colour == 'b')){
			w->colour = 'r';
			if(parent->p == NULL){
				scoreboard_delete_fixup(root, parent, NULL, 'l');
			} else if(parent == parent->p->left){
				scoreboard_delete_fixup(root, parent, parent->p, 'l');
			} else {
				scoreboard_delete_fixup(root, parent, parent->p, 'r');
			}
		} else {
			if(w->left == NULL || w->left->colour == 'b'){
				w->right->colour = 'b';
				w->colour = 'r';
				scoreboard_left_rotate(root, w);
				w = parent->left;
			}
			w->colour = parent->colour;
			parent->colour = 'b';
			w->left->colour = 'b';
			scoreboard_right_rotate(root, parent);
		}
	} else {
		printf("Invalid Side\n");
		return;
	}
}

void scoreboard_delete(scoreboard_entry** root, scoreboard_entry* z){
	//Deletes node and returns its key
	scoreboard_entry* y = NULL;
	scoreboard_entry* x = NULL;
	char side = 'l';
	if(z->left == NULL || z->right == NULL){
		 y = z;
	} else {
		y = scoreboard_tree_successor(z);		
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
	scoreboard_entry* parent = y->p;
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
		scoreboard_delete_fixup(root, x, parent, side);
	}
	z->left = NULL;
	z->right = NULL;
	z->p = NULL;
}

void scoreboard_insert(scoreboard_entry** root, scoreboard_entry* z){
	//Inserts the scoreboard entry z in the tree
	scoreboard_entry* y = NULL;
	scoreboard_entry* x = *root;
	while(x!=NULL){
		y = x;
		if(!scoreboard_entry_compare(z, x)){
			x = x->left;
		} else {
			x = x->right;
		}
	}	
	z->p = y;
	if(y == NULL){
		*root = z;
	} else if(!scoreboard_entry_compare(z, y)) {
		y->left = z;
	} else {
		y->right = z;
	}
	z->left = NULL;
	z->right = NULL;
	z->colour = 'r';
	scoreboard_insert_fixup(root, z);
}

void scoreboard_inorder_tree_walk(scoreboard_entry* node){
	if(node->left != NULL){
		scoreboard_inorder_tree_walk(node->left);
	} else {
		printf("[PRINT] ");
		printf("Hit a NULL node\n");
	}	
	printf("[PRINT] ");
	printf("Printing node with score %d and colour %c\n", node->score, node->colour);
	if(node->right != NULL){
		scoreboard_inorder_tree_walk(node->right);
	} else { 
		printf("[PRINT] ");
		printf("Hit a NULL node\n");
	}
}

void scoreboard_report_inorder_tree_walk(scoreboard_entry* node, char* ent_string, int score_to_search){
	//Used in a report to traverse the scoreboard tree
	//Because the report requires us to print out the entities with maximal score in increasing alphabetical order
	//we only need to do a reverse inorder tree walk until we hit a node with a score inferior to the maximum
	if(node->right != NULL){
		scoreboard_report_inorder_tree_walk(node->right, ent_string, score_to_search);
	}
	if(node->score == score_to_search){
		fputc(SPACE, stdout);
		fputc(DQUOTES, stdout);
		fputs(node->ent->name, stdout);
		fputc(DQUOTES, stdout);
	} else {
		return;
	}
	if(node->left != NULL){
		scoreboard_report_inorder_tree_walk(node->left, ent_string, score_to_search);
	}
}

void scoreboard_preorder_tree_walk(scoreboard_entry* node){
	printf("[PRINT] ");
	printf("Printing node with score %d and colour %c for entity %s\n", node->score, node->colour, node->ent->name);
	if(node->left != NULL){
		scoreboard_preorder_tree_walk(node->left);
	} else {
		printf("[PRINT] ");
		printf("Hit a NULL node\n");
	}	
	if(node->right != NULL){
		scoreboard_preorder_tree_walk(node->right);
	} else { 
		printf("[PRINT] ");
		printf("Hit a NULL node\n");
	}
}

unsigned int hash_jdb2(int num, ...){
	//As a sort of hash, this function works upon multiple strings and yields a number which is a known function of the input strings.
	//Implements the jdb2 hash function
	va_list arglist;
	int i, j;
	char* currString;
	unsigned int currSum = 0;
	va_start(arglist, num);
	for(i=0; i<num; i++){
		currString = va_arg(arglist, char*);	
		size_t length = strlen(currString);
		for(j=0; j<length; j+=2){
			currSum = currSum*33 + (int)currString[j];
		}
	}
	return currSum % ENTITY_HASH_TABLE_SIZE;
}

entity* add_new_entity(entity** hash_table, char* name){
	//Adds a new entity with a given name
	unsigned int position = hash_jdb2(1,name);

	#ifdef VERBOSE
		printf("[DEBUG] ");
		printf("Adding new entity %s in position %d of hash table\n", name, position);
	#endif
		
	entity* new_entity = malloc(sizeof(entity));
	new_entity->name = malloc(strlen(name)+1);
	new_entity->scoreboard_entry_list = NULL;
	new_entity->relation_root = NULL;
	strcpy(new_entity->name, name);
	entity_hash_table_insert(new_entity, position, hash_table);	

	return new_entity;
}

unsigned int get_rel_type_index(relation_type* current_rel_type, rel_type_reference** relation_type_list){
	//Given a rel_type, returns its index
	for(int i = 0; i < REL_TYPE_ARRAY_SIZE; i++){
		if(relation_type_list[i] && relation_type_list[i]->rel_type == current_rel_type){
			return i;
		}
	}
	return 0;
}

relation* add_new_relation(entity* from, entity* to, relation_type* current_rel_type, unsigned int rel_type_index, relation* insertion_point){
	//Inserts a new relation between the entities from and to
	relation* new_relation = malloc(sizeof(relation));
	new_relation->from = from;
	new_relation->rel_type_present = 1 << rel_type_index;
	if(insertion_point){
		relation_tree_precision_insert(&(to->relation_root), new_relation, insertion_point);
	} else {
		//This is the case in which current_rel_type has been newly created
		relation_tree_insert(&(to->relation_root), new_relation, rel_type_index);
	}
	return new_relation;
}

void fix_relation_type_references(relation_type** global_relation_type_list, rel_type_reference** relation_type_reference_list){
	//As the relation type list might get its element shifted upon insertion, we need to update the pointers in the reference list any time a new relation type is inserted
	for(int i = 0; i < REL_TYPE_ARRAY_SIZE; i++){
		if(global_relation_type_list[i]){
			global_relation_type_list[i]->reference->rel_type = global_relation_type_list[i];
		}
	}
}

relation_type* add_new_relation_type(char* name, relation_type** relation_type_list, rel_type_reference** relation_type_reference_list){
	//Inserts a new relation type
	relation_type* new_rel_type = malloc(sizeof(relation_type));
	rel_type_reference* new_rel_type_ref = malloc(sizeof(rel_type_reference));
	new_rel_type->name = malloc(strlen(name)+1);
	strcpy(new_rel_type->name, name);
	new_rel_type->reference = new_rel_type_ref;
	new_rel_type_ref->rel_type = new_rel_type;

	int i;
	for(i = 0; i < REL_TYPE_ARRAY_SIZE; i++){
		if(!relation_type_list[i] || strcmp(relation_type_list[i]->name, name) > 0){
			break;			
		}
	}
	if(relation_type_list[i]){
		memmove(&(relation_type_list[i+1]), &(relation_type_list[i]), sizeof(relation_type*)*(REL_TYPE_ARRAY_SIZE-i-1));
	}
	relation_type_list[i] = new_rel_type;

	for(i = 0; i < REL_TYPE_ARRAY_SIZE; i++){
		if(!relation_type_reference_list[i]){
			relation_type_reference_list[i] = new_rel_type_ref;
			break;
		}
	}
	initialize_new_rel_type(new_rel_type);
	fix_relation_type_references(relation_type_list, relation_type_reference_list);
	return new_rel_type;
}

void delete_scoreboard_entry(scoreboard_entry* current_entry, relation_type* current_rel_type){
	//Deletes the scoreboard entry
	scoreboard_delete(&(current_rel_type->root), current_entry);
	scoreboard_entry_list_delete(current_entry, &(current_entry->ent->scoreboard_entry_list));
	free(current_entry);
}

scoreboard_entry* add_scoreboard_entry(entity* ent, relation_type* current_rel_type, int score){
	//Creates a new scoreboard entry with a set score
	scoreboard_entry* new_node = malloc(sizeof(scoreboard_entry));
	new_node->score = score;
	new_node->ent = ent;
	new_node->rel_type = current_rel_type;
	scoreboard_entry_list_head_insert(new_node, &(ent->scoreboard_entry_list));
	scoreboard_insert(&(current_rel_type->root), new_node);
	return new_node;
}

void update_scoreboard_entry(scoreboard_entry* current_entry, relation_type* current_rel_type, int score_difference){
	//Updates the score of a scoreboard entry
	if(current_entry->score + score_difference <= 0){
		delete_scoreboard_entry(current_entry, current_rel_type);
	} else {
		scoreboard_delete(&(current_rel_type->root), current_entry);
		current_entry->score += score_difference;
		scoreboard_insert(&(current_rel_type->root), current_entry);
	}
}

void delete_entity(entity* to_delete, entity** global_entity_hash_table){
	//Deletes an entity
	unsigned int position = hash_jdb2(1, to_delete->name);
	entity_list_delete(to_delete, &(global_entity_hash_table[position]));
}

int get_number_of_rel_types_in_relation(relation* current_relation){
	//This gets the number of relation types for which current_relation is valid
	//Equivalent to computing the Hamming Weight of rel_types_present
	//Currently using Brian Kernighan's algorithm
	int hamming_weight; 
	unsigned int rel_type_number = current_relation->rel_type_present;
	for(hamming_weight = 0; rel_type_number; rel_type_number &= rel_type_number - 1){
		hamming_weight++;
	}
	return hamming_weight;
}

void remove_rel_type_in_relation(relation* to_delete, unsigned int rel_type_index){
	//Makes the relation not valid for the given relation type, indicated by its index
	to_delete->rel_type_present -= 1 << rel_type_index;
}

void delete_relation(relation* to_delete, entity* ent_to, unsigned int index){
	//Makes the relation not for a given relation type. If it's the only relation type present, it deletes the relation node entirely
	if(get_number_of_rel_types_in_relation(to_delete) == 1){
		relation_tree_delete(&(ent_to->relation_root), to_delete);
	} else {
		remove_rel_type_in_relation(to_delete, index);
	}
}

void initialize_global_structure(entity*** hash_table, relation_type*** relation_type_list, rel_type_reference*** relation_type_reference){
	//Initializes every data structure necessary for the program
	*hash_table = get_new_entity_hash_table();
	*relation_type_list = calloc(REL_TYPE_ARRAY_SIZE, sizeof(relation_type*));
	*relation_type_reference = calloc(REL_TYPE_ARRAY_SIZE, sizeof(relation_type*));
}

void delete_empty_rel_types(relation_type** relation_type_list){
	for(int i = 0; i<REL_TYPE_ARRAY_SIZE; i++){
		if(relation_type_list[i] && relation_type_list[i]->root == NULL){
			free(relation_type_list[i]->name);
			free(relation_type_list[i]);
			relation_type_list[i] = NULL;
			cached_relation_type = NULL;
		}
	}
}

void delete_relation_in_entity(entity* ent_to, entity* to_delete, rel_type_reference** relation_type_reference_list){
	//Deletes the relation that connects to_delete to ent_to, making it not valid for any relation type and deleting the relation node
	relation* current_entry = relation_tree_search(ent_to->relation_root, to_delete, NULL);
	if(current_entry){
		scoreboard_entry* current_scoreboard_entry;
		for(int i = 0; i < REL_TYPE_ARRAY_SIZE; i++){
			if(current_entry->rel_type_present & (1 << i)){
				current_scoreboard_entry = find_scoreboard_entry(relation_type_reference_list[i]->rel_type, ent_to);
				if(current_scoreboard_entry){
					update_scoreboard_entry(current_scoreboard_entry, relation_type_reference_list[i]->rel_type, -1);
				}
			}
		}
		relation_tree_delete(&(ent_to->relation_root), current_entry);
	}
}

void clear_invalid_entries(entity** global_entity_hash_table, rel_type_reference** rel_list, entity* to_delete){
	//Deletes all entries that connect to_delete to any entity, to remove any reference to it
	for(int i = 0; i < ENTITY_HASH_TABLE_SIZE; i++){
		if(global_entity_hash_table[i]){
			#ifdef VERBOSE
				printf("[DEBUG] ");
				printf("Removing invalid entries in list starting with %s\n", global_entity_hash_table[i]->name);
			#endif
			entity* nav = global_entity_hash_table[i];
			while(nav){
				if(nav->relation_root){
					delete_relation_in_entity(nav, to_delete, rel_list);
				}
				nav = nav->next;
			}
		}
	}
}

void initialize_new_rel_type(relation_type* new_rel_type){
	new_rel_type->root = NULL;
}

void addent(char* name, entity** global_entity_hash_table, relation_type** global_relation_type_list){
	#ifdef VERBOSE
		printf("[DEBUG] ");
		printf("addent: entity %s\n", name);
	#endif

	entity* current_entity;

	#ifdef CACHE_ENABLED
		if(cached_entity_A && strcmp(cached_entity_A->name, name)==0){
			#ifdef VERBOSE
				printf("[DEBUG] ");
				printf("Using cached entity %s\n", name); 
			#endif
			current_entity = cached_entity_A;	
		} else {
			current_entity = find_entity(name, global_entity_hash_table);
			if(current_entity){
				cached_entity_A = current_entity;
			}
		}
	#else
		current_entity = find_entity(name, global_entity_hash_table);
	#endif

	if(!current_entity){
		//If entity does not already exist then add it
		add_new_entity(global_entity_hash_table, name);
	}
}

void delent(char* name, entity** global_entity_hash_table, relation_type** global_relation_type_list, rel_type_reference** relation_type_reference_list){
	#ifdef VERBOSE
		printf("[DEBUG] ");
		printf("delent: entity %s\n", name);
	#endif
	//Check if entity actually exists and tries to fetch it
	entity* current_entity;
	#ifdef CACHE_ENABLED
		if(cached_entity_A && strcmp(cached_entity_A->name, name)==0){
			#ifdef VERBOSE
				printf("[DEBUG] ");
				printf("Using cached entity %s\n", name); 
			#endif
			current_entity = cached_entity_A;	
			cached_entity_A = NULL;
		} else {
			current_entity = find_entity(name, global_entity_hash_table);
		}
		if(cached_entity_B && strcmp(cached_entity_B->name, name)==0){
			cached_entity_B = NULL;
		}
	#else
		current_entity = find_entity(name, global_entity_hash_table);
	#endif

	if(current_entity){
		//If the entity exists, then delete every reference to this entity from other entities
		for(int i = 0; i < REL_TYPE_ARRAY_SIZE; i++){
			//For each relation type, find and delete the scoreboard entry (if it exists) associated with the entity
			scoreboard_entry* current_scoreboard_entry = find_scoreboard_entry(global_relation_type_list[i], current_entity);
			if(current_scoreboard_entry){
				delete_scoreboard_entry(current_scoreboard_entry, global_relation_type_list[i]);	
			}
		}
		//Clear up any reference to the entity and delete it
		clear_invalid_entries(global_entity_hash_table, relation_type_reference_list, current_entity);
		delete_entity(current_entity, global_entity_hash_table);
	}
}

void addrel(char* from, char* to, char* name, entity** global_entity_hash_table, relation_type** global_relation_type_list, rel_type_reference** relation_type_reference_list){
	#ifdef VERBOSE
		printf("[DEBUG] ");
		printf("addrel: from %s to %s. Type: %s\n", from, to, name);
	#endif

	//Find if both entities that are involved exist
	entity *ent_from = NULL, *ent_to = NULL;
	relation* insertion_point = NULL;

	#ifdef CACHE_ENABLED
		if(cached_entity_A){
			if(strcmp(cached_entity_A->name, from)==0){
				#ifdef VERBOSE
					printf("[DEBUG] ");
					printf("Using cached entity A as from: %s\n", cached_entity_A->name); 
				#endif
				ent_from = cached_entity_A;
			}
			if(strcmp(cached_entity_A->name, to)==0){
				#ifdef VERBOSE
					printf("[DEBUG] ");
					printf("Using cached entity A as to: %s\n", cached_entity_A->name); 
				#endif
				ent_to = cached_entity_A;
			}
		}
		if(cached_entity_B && (!ent_from || !ent_to)){
			if(strcmp(cached_entity_B->name, from)==0){
				#ifdef VERBOSE
					printf("[DEBUG] ");
					printf("Using cached entity B as from: %s\n", cached_entity_B->name); 
				#endif
				ent_from = cached_entity_B;
			}
			if(strcmp(cached_entity_B->name, to)==0){
				#ifdef VERBOSE
					printf("[DEBUG] ");
					printf("Using cached entity B as to: %s\n", cached_entity_B->name); 
				#endif
				ent_to = cached_entity_B;
			}
		}
		if(!ent_from) {
			ent_from  = find_entity(from, global_entity_hash_table);
			if(ent_from){
				#ifdef VERBOSE
					printf("[DEBUG] ");
					printf("Overwriting cached entity A with %s\n", ent_from->name); 
				#endif
				cached_entity_A = ent_from;
			}
		}
		if(!ent_to){
			ent_to  = find_entity(to, global_entity_hash_table);
			if(ent_to){
				#ifdef VERBOSE
					printf("[DEBUG] ");
					printf("Overwriting cached entity B with %s\n", ent_to->name); 
				#endif
				cached_entity_B = ent_to;
			}
		}
	#else
		ent_from = find_entity(from, global_entity_hash_table);
		ent_to = find_entity(to, global_entity_hash_table);
	#endif

	if(ent_from && ent_to){
		//If both entities actually exist then attempt an insertion of a new relation between ent_from and ent_to
		relation_type* current_rel_type;
		relation* current_relation = NULL;
		if(CACHE_ENABLED){
			if(cached_relation_type && strcmp(cached_relation_type->name, name)==0){
				#ifdef VERBOSE
					printf("[DEBUG] ");
					printf("Using cached relation type: %s\n", cached_relation_type->name); 
				#endif
				current_rel_type = cached_relation_type;
			} else {
				current_rel_type = find_relation_type(name, global_relation_type_list);
				if(current_rel_type){
					#ifdef VERBOSE
						printf("[DEBUG] ");
						printf("Overwriting cached relation type with %s\n", current_rel_type->name); 
					#endif
					cached_relation_type = current_rel_type;
				}
			}
		} else {
			current_rel_type = find_relation_type(name, global_relation_type_list);
		}
		if(!current_rel_type){
			//If the current relation type does not exist then create it
			current_rel_type = add_new_relation_type(name, global_relation_type_list, relation_type_reference_list);	
			unsigned int index = get_rel_type_index(current_rel_type, relation_type_reference_list);
			//Create a new entry for the scoreboard
			add_scoreboard_entry(ent_to, current_rel_type, 1);
			//Add the relation
			current_relation = find_relation(ent_from, ent_to, current_rel_type, &insertion_point);
			if(current_relation){
				add_rel_type_to_relation(current_relation, index);
			} else {
				add_new_relation(ent_from, ent_to, current_rel_type, index, insertion_point);
			}
		} else {
			unsigned int index = get_rel_type_index(current_rel_type, relation_type_reference_list);
			//If the current relation type does exist...
			current_relation = find_relation(ent_from, ent_to, current_rel_type, &insertion_point); 
			if(current_relation && has_matching_rel_type(current_relation, index)){
				//Check if the relation already exists: if so, exit the function
				return;
			} else if (current_relation){
				//Otherwise, if the relation between those same two entities already exists, yet without the correct rel_type, add it
				add_rel_type_to_relation(current_relation, index);
			} else {
				//Finally, if there is no relation between those two entities, add the relation
				add_new_relation(ent_from, ent_to, current_rel_type, index, insertion_point);
			}
			//Check if there's an existing scoreboard entry for the receiving entity
			scoreboard_entry* current_scoreboard_entry = find_scoreboard_entry(current_rel_type, ent_to);
			if(current_scoreboard_entry){
				//If the scoreboard entry exists, then update it, adding 1 to its score
				update_scoreboard_entry(current_scoreboard_entry, current_rel_type, 1);
			} else {
				//Otherwise create a new scoreboard entry with score 1
				add_scoreboard_entry(ent_to, current_rel_type, 1);
			}
		}
	}
}

void delrel(char* from, char* to, char* name, entity** global_entity_hash_table, relation_type** global_relation_type_list, rel_type_reference** relation_type_reference_list){
	//Check if relation type exists
	#ifdef VERBOSE
		printf("[DEBUG] ");
		printf("delrel: from %s to %s. Type: %s\n", from, to, name);
	#endif

	relation_type* current_rel_type = find_relation_type(name, global_relation_type_list);
	if(current_rel_type){
		entity *ent_from = NULL, *ent_to = NULL;
		#ifdef CACHE_ENABLED
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
				ent_from  = find_entity(from, global_entity_hash_table);
				if(ent_from){
					cached_entity_A = ent_from;
				}
			}
			if(!ent_to){
				ent_to  = find_entity(to, global_entity_hash_table);
				if(ent_to){
					cached_entity_B = ent_to;
				}
			}
		#else
			ent_from = find_entity(from, global_entity_hash_table);
			ent_to = find_entity(to, global_entity_hash_table);
		#endif

		//If so, find both entities involved
		if(ent_from && ent_to){
			//If both entities exist, then check if the relation that we need to delete exists
			relation* current_relation = find_relation(ent_from, ent_to, current_rel_type, NULL);
			unsigned int index = get_rel_type_index(current_rel_type, relation_type_reference_list);
			if(!current_relation || !has_matching_rel_type(current_relation, index)){
				//If no such relation exists, then do nothing
				return;
			}
			//Otherwise we need to actually delete the relation: update the scoreboard, removing 1 from the score
			scoreboard_entry* current_scoreboard_entry = find_scoreboard_entry(current_rel_type, ent_to);
			update_scoreboard_entry(current_scoreboard_entry, current_rel_type, -1);
			//Delete the relation object
			delete_relation(current_relation, ent_to, index);
		}
	}
}

int is_rel_type_array_empty(relation_type** global_relation_type_list){
	for(int i = 0; i < REL_TYPE_ARRAY_SIZE; i++){
		if(global_relation_type_list[i]){
			return 0;
		}
	}
	return 1;
}

void report(entity** global_entity_hash_table, relation_type** global_relation_type_list){
	delete_empty_rel_types(global_relation_type_list);
	int first_line = 1;
	int i;
	if(is_rel_type_array_empty(global_relation_type_list)){
		puts("none");
		return;
	} 
	for(i=0; i<REL_TYPE_ARRAY_SIZE; i++){
		if(global_relation_type_list[i] == NULL){
			continue;
		} else {
			if(!first_line){
				//Adding space after semicolon if needed
				fputc(SPACE, stdout);
			}
			fputc(DQUOTES, stdout);
			fputs(global_relation_type_list[i]->name, stdout);
			fputc(DQUOTES, stdout);
			scoreboard_entry* max_node = scoreboard_tree_maximum(global_relation_type_list[i]->root);
			scoreboard_report_inorder_tree_walk(global_relation_type_list[i]->root, current_line, max_node->score);
			fputc(SPACE, stdout);
			fprintf(stdout, "%d", max_node->score);
			fputc(SEMICOLON, stdout);
			if(first_line){
				first_line = 0;
			}
		}
	}
	fputc(RETURN_CARRIAGE, stdout);
}

int main(){
	entity** global_entity_hash_table;
	relation_type** global_relation_type_list;
	rel_type_reference** relation_type_reference_list;
	initialize_global_structure(&global_entity_hash_table, &global_relation_type_list, &relation_type_reference_list);
	char *inputBuffer = malloc(INPUT_BUFFER_SIZE);
	size_t n = INPUT_BUFFER_SIZE;
	char *command = NULL, *arg1 = NULL, *arg2 = NULL, *arg3 = NULL;
	char separators[] = " \"\n";
	while(1){
		if(fgets(inputBuffer, n, stdin)){
			if(inputBuffer[0]=='e'){ 
				//The command is [e]nd
				return 0;
			} 
			command = strtok(inputBuffer, separators);
			if(command[0]=='r'){
				//The command is [r]eport
				report(global_entity_hash_table, global_relation_type_list);
			} else if(command[0]=='a'){
				//The command is an [a]dd
				if(command[3]=='e'){
					//[a]dd[e]nt
					arg1 = strtok(NULL, separators);	
					addent(arg1, global_entity_hash_table, global_relation_type_list);
				} else if(command[3]=='r'){
					//[a]dd[r]el
					arg1 = strtok(NULL, separators);
					arg2 = strtok(NULL, separators);
					arg3 = strtok(NULL, separators);
					addrel(arg1, arg2, arg3, global_entity_hash_table, global_relation_type_list, relation_type_reference_list);
				}
			} else if (command[0] == 'd'){
				//The command is a [d]el
				if(command[3]=='e'){
					//[d]el[e]nt
					arg1 = strtok(NULL, separators);
					delent(arg1, global_entity_hash_table, global_relation_type_list, relation_type_reference_list);
				} else if(command[3]=='r'){
					//[d]el[r]el
					arg1 = strtok(NULL, separators);
					arg2 = strtok(NULL, separators);
					arg3 = strtok(NULL, separators);
					delrel(arg1, arg2, arg3, global_entity_hash_table, global_relation_type_list, relation_type_reference_list);
				}
			}
		} else {
			#ifdef VERBOSE
				printf("Failed to read line: terminating program\n");
			#endif
			return 0;
		}
	}
	return 0;
}
