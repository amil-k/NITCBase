#include "OpenRelTable.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdlib.h>


OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];
/* THis is responsible for initializing and managing relation cache and attribute cache*/

  // initialize relCache and attrCache with nullptr

OpenRelTable::OpenRelTable() {

 
    for (int i = 0; i < MAX_OPEN; ++i) {
      RelCacheTable::relCache[i] = nullptr;
      AttrCacheTable::attrCache[i] = nullptr;
      tableMetaInfo[i].free=true;
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
  
  // iterate through all the attributes of the relation catalog and create a linked
  for(int i=0;i< RELCAT_NO_ATTRS;i++){
    attrCatBlock.getRecord(attrCatRecord, i);
    AttrCacheEntry *entry = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
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

    for (int i = 0; i < ATTRCAT_NO_ATTRS; i++) {

        int slot = RELCAT_NO_ATTRS + i;
        attrCatBlock.getRecord(attrCatRecord,slot);
        AttrCacheEntry *entry =(AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

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





  /************ Setting up tableMetaInfo entries ************/

  // in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  //   set relname for RELCAT_RELID and ATTRCAT_RELID

  tableMetaInfo[RELCAT_RELID].free =false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName ,RELCAT_RELNAME);

  tableMetaInfo[ATTRCAT_RELID].free =false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName ,ATTRCAT_RELNAME);



/* setting up students relation*/

    /* finding the relation catalog slot*/
//   HeadInfo relCatHeader;
//   relCatBlock.getHeader(&relCatHeader);
//   int slot=-1;
//   int numRels=relCatHeader.numEntries;
//   for(int i=0;i<numRels;i++){
//     // Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog
//     relCatBlock.getRecord(relCatRecord, i);
//     if(strcmp("Students", relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0){
//       slot=i;
//       break;
//     }
//   }

//   if(slot!=-1){
//     RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
//     relCacheEntry.recId.block=RELCAT_BLOCK;
//     relCacheEntry.recId.slot=slot;
//     RelCacheTable::relCache[ATTRCAT_RELID+1] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
//     *(RelCacheTable::relCache[ATTRCAT_RELID+1]) = relCacheEntry;
//   }else std::cout<<"\n\n\nRelation NOT FOUND\n\n\n\n";

//   /* corresponding attr catalog s*/
//   head = nullptr;
//   prev = nullptr;
//   AttrCacheEntry *temp = nullptr;

//   if(slot!=-1){



//     head=(AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
//     temp=head; 
//     int numAttrs=RelCacheTable::relCache[ATTRCAT_RELID+1]->relCatEntry.numAttrs; 
//     for(int i=12;i<12+numAttrs;i++){
//       // iterate through all the attributes of the relation catalog and create a linked
//       attrCatBlock.getRecord(attrCatRecord, i); // slots 6-11 are for attribute catalog attributes
//       AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &temp->attrCatEntry);  

//       //std::cout << "Students cache: "<< temp->attrCatEntry.attrName << std::endl;


//       temp->recId.block=ATTRCAT_BLOCK;
//       temp->recId.slot=i;

//       if(i<12+numAttrs-1)
//         temp->next=(AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
//       else
//         temp->next=nullptr;
//       temp=temp->next;
//     } 
//     AttrCacheTable::attrCache[ATTRCAT_RELID+1] = head /* head of the linked list */;
//   }

}

OpenRelTable::~OpenRelTable() {



  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  // free the memory allocated for rel-id 0 and 1 in the caches

  for (int i = 0; i <= 1; i++) {
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

int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/

  for(int relId=0;relId<MAX_OPEN;relId++){

    //std::cout << relId << " : " << tableMetaInfo[relId].free << std::endl;
    if(tableMetaInfo[relId].free == true) return relId;
  }
  // if found return the relation id, else return E_CACHEFULL.
  return E_CACHEFULL;
}



int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/

    for(int relId=0;relId<MAX_OPEN;relId++){
      if(strcmp(tableMetaInfo[relId].relName,relName)==0){
        return relId;
      }
    }
    return E_RELNOTOPEN;

  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  int relId = OpenRelTable::getRelId(relName);
  if(relId>=0   /* the relation `relName` already has an entry in the Open Relation Table */){
    // (checked using OpenRelTable::getRelId())
    // return that relation id;
    return relId;
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */

  else{
    relId = OpenRelTable::getFreeOpenRelTableEntry();
  }


  if (relId == E_CACHEFULL/* free slot not available */){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.


  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

  RelCacheTable::resetSearchIndex(RELCAT_RELID);


  char RelCat_Attr_RelName[ATTR_SIZE];
  strcpy(RelCat_Attr_RelName, RELCAT_ATTR_RELNAME);

  Attribute AttrVal;
  strcpy(AttrVal.sVal,relName);
  

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID,RelCat_Attr_RelName,AttrVal,EQ);
  if (relcatRecId.block == -1 && relcatRecId.slot == -1   /* relcatRecId == {-1, -1} */) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
  RecBuffer recEntryBlock(relcatRecId.block);
  Attribute recEntrySlot[RELCAT_NO_ATTRS];
  recEntryBlock.getRecord(recEntrySlot,relcatRecId.slot);

  struct RelCacheEntry recCacheEntry;
  RelCacheTable::recordToRelCatEntry(recEntrySlot,&recCacheEntry.relCatEntry);

  recCacheEntry.recId.block = relcatRecId.block;
  recCacheEntry.recId.slot = relcatRecId.slot;

  RelCacheTable::relCache[relId] =(struct RelCacheEntry*) malloc(sizeof (RelCacheEntry));
  *(RelCacheTable::relCache[relId]) = recCacheEntry;


  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.

  AttrCacheEntry* attrCacheEntry = (AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
  AttrCacheEntry* listHead = attrCacheEntry;

  int numAttr = recCacheEntry.relCatEntry.numAttrs;
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);


  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/


  char AttrCat_Attr_RelName[ATTR_SIZE];
  strcpy(AttrCat_Attr_RelName, ATTRCAT_ATTR_RELNAME);

  for(int i=0;i<numAttr;i++)
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID,AttrCat_Attr_RelName,AttrVal,EQ);
      
      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()

    RecBuffer recBuffer(attrcatRecId.block);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    recBuffer.getRecord(attrCatRecord,attrcatRecId.slot);

    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&attrCacheEntry->attrCatEntry);
    attrCacheEntry->recId = attrcatRecId;
    if(i==numAttr-1){
        attrCacheEntry->next=nullptr;
    }
    else{
      attrCacheEntry->next = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
      attrCacheEntry= attrCacheEntry->next;
    }

  }

  // set the relIdth entry of the AttrCacheTable to listHead.
AttrCacheTable::attrCache[relId] = listHead;
  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  tableMetaInfo[relId].free =false;
  strcpy(tableMetaInfo[relId].relName,relName);

  return relId;
}


int OpenRelTable::closeRel(int relId) {
  if (relId == 0 || relId == 1 /* rel-id corresponds to relation catalog or attribute catalog*/) {
    return E_NOTPERMITTED;
  }

  if (relId < 0 || relId > MAX_OPEN /* 0 <= relId < MAX_OPEN */) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free == true/* rel-id corresponds to a free slot*/) {
    return E_RELNOTOPEN;
  }

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function

// freeing relcache
  free(RelCacheTable::relCache[relId]);
  RelCacheTable::relCache[relId] = nullptr;
// freeing attrCache
  AttrCacheEntry * attrCacheEntry = AttrCacheTable::attrCache[relId];
  AttrCacheEntry* head = attrCacheEntry;
  AttrCacheEntry* next = attrCacheEntry->next;
  while(next!=NULL){
    attrCacheEntry = next;
    next= next->next;
    free(attrCacheEntry);
  }
  free(head);
  AttrCacheTable::attrCache[relId] = nullptr;

  


  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr
  tableMetaInfo[relId].free=true;

  return SUCCESS;
}
