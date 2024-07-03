#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table){
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table){
    FREE_ARRAY(Entry,table->entries,table->capacity);
    initTable(table);
}

static Entry* findEntry(Entry* entries,int capacity,ObjString* key){
    // basically if the ideal location 'index' is filled then we search for some place
    // this function only returns if it finds a bucket empty or filled with the curr
    // ObjString->key
    uint32_t index = key->hash % capacity;
    for(;;){
        Entry* entry  = &entries[index];
        Entry* tombstone = NULL;
        // if the key is NULL , before bailing out we search if it is a tombstone
        if(entry->key == NULL){
            // if the value is NIL , then it isnt a tomstone 
            // since tomstones never have NIL_VAL
            // else it is a tombstone is and we select as a tombstone
            if(IS_NIL(entry->value)){
                return tombstone != NULL ? tombstone: entry;
            }else{
                if(tombstone == NULL) tombstone = entry;
            }
        }else if(entry->key == key){
            return entry;
        }

        index = (index+1) % capacity;
    }
}

static void adjustCapacity(Table* table,int capacity){
    // alloacte a new table and make every bucket NULL and NIL_VAL by default
    Entry* new_entry = ALLOCATE(Entry,capacity);
    for(int i = 0;i < capacity;i++){
        new_entry[i].key = NULL;
        new_entry[i].value = NIL_VAL;
    }

    /*
        loop over the main array and then
        if the key if not null
        find a dest for it in the new table
        note that findEntry() uses 'entries' which is the new hash table
    */
    table->count = 0;
    for(int i = 0;i < table->capacity;i++){
        Entry* entry = &table->entries[i];
        if(entry->key == NULL) continue;

        Entry* dest = findEntry(new_entry,capacity,entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry,table->entries,table->capacity);
    table->entries = new_entry;
    table->capacity = capacity;
}


/*
instead of just getting the value
if it does find the value it will update it to 'value' param accordingly
*/
bool tableGet(Table* table,ObjString* key,Value* value){
    if (table->count == 0) return false;
    Entry* entry = findEntry(table->entries,table->capacity,key);
    if(entry->key == NULL) return false;
    *value = entry->value;
    return true;
}

bool tableSet(Table* table,ObjString* key,Value value){
    // if we are bigger than the load factor (0.75) 
    // than we grow the array
    if(table->count +1 > table->capacity * TABLE_MAX_LOAD){
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table,capacity);
    }
    Entry* entry = findEntry(table->entries,table->capacity,key);

    bool is_new_key = entry->key == NULL;
    // the key being NULL isnt the only thing to mark that it is new
    // the real mark is if it also NIL in value
    if(is_new_key && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return is_new_key;
}


bool tableDelete(Table* table,ObjString* key){
    if(table->count == 0) return false;

    Entry* entry = findEntry(table->entries,table->capacity,key);
    if(entry->key == NULL) return false;


    entry->key = NULL;
    // BOOL_VAL(true) acts as a tombstone
    entry->value = BOOL_VAL(true);

    return true;
}

void tableAddAll(Table* from,Table* to){
    for(int i = 0;i < from->capacity;i++){
        Entry* entry = &from->entries[i];
        if(entry->key == NULL){
            tableSet(to,entry->key,entry->value);
        }
    }
}


ObjString* tableFindString(Table* table,const char* chars,int length,uint32_t hash){
    if(table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;

    for(;;){
        Entry* entry = &table->entries[index];
        if(entry->key == NULL){
            if(IS_NIL(entry->value)) return NULL;
        }else if(
            entry->key->length == length 
            && entry->key->hash == hash 
            && memcmp(entry->key->chars,chars,length) == 0
            ){
            return entry->key;
        }

        index = (index+1) % table->capacity;
    }
}