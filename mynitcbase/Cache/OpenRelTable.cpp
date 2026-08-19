#include "OpenRelTable.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdlib.h>

/* THis is responsible for initializing and managing relation cache and attribute cache*/

OpenRelTable::OpenRelTable() {

 
  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.) 

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT); /* this is slot zero corresponding the relation relation catalogue itself*/

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry); /* why we convert it is because the size of both the things are different*/
  
  
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*) malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/
  /* this corresponds to the slot 1 in relation catalog, i.e the attribute catalog relation*/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  
  Attribute attrCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(attrCatRecord,RELCAT_SLOTNUM_FOR_ATTRCAT);
  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheEntry attrCatRelCacheEntry;

  RelCacheTable::recordToRelCatEntry(attrCatRecord,&attrCatRelCacheEntry.relCatEntry);

  attrCatRelCacheEntry.recId.block = RELCAT_BLOCK;
  attrCatRelCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  // that is the second entry inside the relcache is the atrribute catalog relation
  RelCacheTable::relCache[ATTRCAT_RELID] = (RelCacheEntry*) malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatRelCacheEntry;


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)



  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);


  AttrCacheEntry *head = nullptr;
  AttrCacheEntry *prev = nullptr;
  AttrCacheEntry *entry = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));  
  // iterate through all the attributes of the relation catalog and create a linked
  for(int i=0;i< RELCAT_NO_ATTRS;i++){
    attrCatBlock.getRecord(attrCatRecord, i);
    
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&entry->attrCatEntry);
  // list of AttrCacheEntry (slots 0 to 5)

  // for each of the entries, set
    entry->recId.block = ATTRCAT_BLOCK;
    entry->recId.slot = i;
    entry->dirty = false;
    entry->searchIndex = {-1, -1};

  // NOTE: allocate each entry dynamically using malloc
  // set the next field in the last entry to nullptr
  // First node
    entry->next = nullptr;
    if (head == nullptr) {
        head = entry;
    }
    else {
        prev->next = entry;
    }

    prev = entry;
  }
  AttrCacheTable::attrCache[RELCAT_RELID] = head;/* head of the linked list */

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  head = nullptr;
  prev = nullptr;
  // AttrCacheEntry *entry =(AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  
    for (int i = 0; i < ATTRCAT_NO_ATTRS; i++) {

        int slot = RELCAT_NO_ATTRS + i;
        attrCatBlock.getRecord(attrCatRecord,slot);
        

        AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&entry->attrCatEntry);

        entry->recId.block = ATTRCAT_BLOCK;
        entry->recId.slot = slot;
        entry->dirty = false;
        entry->searchIndex = {-1, -1};
        entry->next = nullptr;

        if (head == nullptr) {
            head = entry;
        }
        else {
            prev->next = entry;
        }

        prev = entry;
    }

    AttrCacheTable::attrCache[ATTRCAT_RELID] = head;









//assignment part relational catalog cache

  HeadInfo relCatHeader;
  relCatBlock.getHeader(&relCatHeader);
  int slot=-1;
  int numRels=relCatHeader.numEntries;
  for(int i=0;i<numRels;i++){
    // Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog
    relCatBlock.getRecord(relCatRecord, i);
    if(strcmp("Students", relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0){
      slot=i;
      break;
    }
  }

  if(slot!=-1){
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block=RELCAT_BLOCK;
    relCacheEntry.recId.slot=slot;
    RelCacheTable::relCache[ATTRCAT_RELID+1] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID+1]) = relCacheEntry;
  }else std::cout<<"\n\n\nRelation NOT FOUND\n\n\n\n";

//attribute catalog cache
  head = nullptr;
  prev = nullptr;
  AttrCacheEntry *temp = nullptr;

  if(slot!=-1){
    head=(AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
    temp=head; 
    int numAttrs=RelCacheTable::relCache[ATTRCAT_RELID+1]->relCatEntry.numAttrs; 
    for(int i=12;i<12+numAttrs;i++){
      // iterate through all the attributes of the relation catalog and create a linked
      attrCatBlock.getRecord(attrCatRecord, i); // slots 6-11 are for attribute catalog attributes
      AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &temp->attrCatEntry);  
      temp->recId.block=ATTRCAT_BLOCK;
      temp->recId.slot=i;

      if(i<12+numAttrs-1)
        temp->next=(AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
      else
        temp->next=nullptr;
      temp=temp->next;
    } 
    AttrCacheTable::attrCache[ATTRCAT_RELID+1] = head /* head of the linked list */;
  }











}

OpenRelTable::~OpenRelTable() {
  // free all the memory that you allocated in the constructor
  for (int i = 0; i < MAX_OPEN; i++) {
    if (RelCacheTable::relCache[i] != nullptr) {
        free(RelCacheTable::relCache[i]);
        RelCacheTable::relCache[i] = nullptr;
    }
  }

  for (int i = 0; i < MAX_OPEN; i++) {
    AttrCacheEntry* current = AttrCacheTable::attrCache[i];
    while (current != nullptr) {
        AttrCacheEntry* next = current->next;
        free(current);
        current = next;
    }

    AttrCacheTable::attrCache[i] = nullptr;
}


}