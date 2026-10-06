#include "BlockAccess.h"
#include <cstring>
#include <iostream>
#include "Logger/logger.h"

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    // get the previous search index of the relation relId from the relation cache
    // (use RelCacheTable::getSearchIndex() function)
    // let block and slot denote the record id of the record being currently checked
    // if the current search index record is invalid(i.e. both block and slot = -1)
    
    RecId prevRecId;
    int ret = RelCacheTable::getSearchIndex(relId,&prevRecId);
    int block;
    int slot;

    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (no hits from previous search; search should start from the
        // first record itself)
        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        // block = first record block of the relation
        // slot = 0

        RelCatEntry relCatBuffer;

       
        ret = RelCacheTable::getRelCatEntry(relId,&relCatBuffer);
         /********extra*********************** */
        // std::cout << "Students cache:"
        //   << " firstBlk=" << relCatBuffer.firstBlk
        //   << " lastBlk=" << relCatBuffer.lastBlk
        //   << " numRecs=" << relCatBuffer.numRecs
        //   << std::endl;


        block = relCatBuffer.firstBlk;
        slot = 0;
    }
    else
    {
        // (there is a hit from previous search; search should start from
        // the record next to the search index record)
        // block = search index's block
        // slot = search index's slot + 1

      block = prevRecId.block;
      slot = prevRecId.slot+1;
    }

    /* The following code searches for the next record in the relation
       that satisfies the given condition
       We start from the record id (block, slot) and iterate over the remaining
       records of the relation
    */
    while (block != -1)
    {
        /* create a RecBuffer object for block (use RecBuffer Constructor for
           existing block) */
        // get the record with id (block, slot) using RecBuffer::getRecord()
        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function
        // If slot >= the number of slots per block(i.e. no more slots in this block)

        RecBuffer  recBuffer(block);
        HeadInfo head;
        ret = recBuffer.getHeader(&head);
        
        int noSlots = head.numSlots;
        int noAttr = head.numAttrs;
        unsigned char *slotmap = new unsigned char[noSlots];
        recBuffer.getSlotMap(slotmap);

        
        if(slot >=noSlots)
        {
            // update block = right block of block
            // update slot = 0
            block = head.rblock;
            slot =0;
            continue;  // continue to the beginning of this while loop
        }

          // if slot is free skip the loop
          // (i.e. check if slot'th entry in slot map of block contains SLOT_UNOCCUPIED)
        if(slotmap[slot] == SLOT_UNOCCUPIED)
        {
            // increment slot and continue to the next record slot
            slot++;
            continue;
        }

        // compare record's attribute value to the the given attrVal as below:
        /*
            firstly get the attribute offset for the attrName attribute
            from the attribute cache entry of the relation using
            AttrCacheTable::getAttrCatEntry()
        */
        /* use the attribute offset to get the value of the attribute from
           current record */
        AttrCatEntry attrCatEntry;
        ret=AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);        

        //find offset to current entry;
        int offset = attrCatEntry.offset;
        Attribute *record = new Attribute[noAttr];
        recBuffer.getRecord(record,slot);
        

       // will store the difference between the attributes
        // set cmpVal using compareAttrs()


        int cmpVal=compareAttrs(record[offset], attrVal, attrCatEntry.attrType); 

        //extra
        // std::cout << "block = " << block
        //   << ", slot = " << slot
        //   << ", attr = " << attrCatEntry.attrName
        //   << ", value = ";

        // if (attrCatEntry.attrType == STRING)
        //     std::cout << record[offset].sVal;
        // else
        //     std::cout << record[offset].nVal;

        // std::cout << ", searchValue = ";

        // if (attrCatEntry.attrType == STRING)
        //     std::cout << attrVal.sVal;
        // else
        //     std::cout << attrVal.nVal;

        // std::cout << ", cmpVal = " << cmpVal << std::endl;



        /* Next task is to check whether this record satisfies the given condition.
           It is determined based on the output of previous comparison and
           the op value received.
           The following code sets the cond variable if the condition is satisfied.
        */
        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {
            /*
            set the search index in the relation cache as
            the record id of the record that satisfies the given condition
            (use RelCacheTable::setSearchIndex function)
            */
          RecId newIndex={block, slot};
          RelCacheTable::setSearchIndex(relId,&newIndex);
          
          return RecId{block, slot};
        }

        slot++;
    }

    // no record in the relation with Id relid satisfies the given condition
    return RecId{-1, -1};

}



int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

      // set newRelationName with newName

    // search the relation catalog for an entry with "RelName" = newRelationName

    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;
    Attribute newRelationName;
    strcpy(newRelationName.sVal,newName);
    RecId searchRelCatForNewRelName = linearSearch(RELCAT_RELID,RELCAT_ATTR_RELNAME,newRelationName,EQ);

    fprintf(logFile, "BlockAccess::renameRelation::LinearSearch() search RELATIONCAT for New Relation Name gave {%d,%d}\n",
        searchRelCatForNewRelName.block,searchRelCatForNewRelName.slot
    );
    fflush(logFile);
    
    if(searchRelCatForNewRelName.block!=-1 && searchRelCatForNewRelName.slot!=-1){
        return E_RELEXIST;
    }

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    // set oldRelationName with oldName

    // search the relation catalog for an entry with "RelName" = oldRelationName

    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;

    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute oldRelationName;    
    strcpy(oldRelationName.sVal,oldName);
    RecId searchRelCatForOldRelName = linearSearch(RELCAT_RELID,RELCAT_ATTR_RELNAME,oldRelationName,EQ);

    fprintf(logFile, "BlockAccess::renameRelation::LinearSearch() search RELATIONCAT for Old Relation Name gave {%d,%d}\n",
        searchRelCatForOldRelName.block,searchRelCatForOldRelName.slot
    );
    fflush(logFile);

    if(searchRelCatForOldRelName.block==-1 && searchRelCatForOldRelName.slot==-1){
        return E_RELNOTEXIST;
    }


    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord

    Attribute record[RELCAT_NO_ATTRS];
    RecBuffer relcatentry(searchRelCatForOldRelName.block);
    relcatentry.getRecord(record,searchRelCatForOldRelName.slot);

    int numAttr = record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    strcpy(record[RELCAT_REL_NAME_INDEX].sVal,newName);
    relcatentry.setRecord(record,searchRelCatForOldRelName.slot);


    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */
    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */

    //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord



    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    
    for(int i=0;i<numAttr;i++){
        RecId searchAttrCatforOldRelName  = linearSearch(ATTRCAT_RELID,ATTRCAT_ATTR_RELNAME,oldRelationName,EQ);

        Attribute attrcatrecord[ATTRCAT_NO_ATTRS];
        RecBuffer attrcatentry(searchAttrCatforOldRelName.block);

        attrcatentry.getRecord(attrcatrecord,searchAttrCatforOldRelName.slot);
        strcpy(attrcatrecord[ATTRCAT_REL_NAME_INDEX].sVal,newName);
        attrcatentry.setRecord(attrcatrecord,searchAttrCatforOldRelName.slot);
    }


    return SUCCESS;
}



int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    // set relNameAttr to relName


    // Search for the relation with name relName in relation catalog using linearSearch()
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr;   
    strcpy(relNameAttr.sVal,relName);


    RecId searchRelCatForRelName = linearSearch(RELCAT_RELID,RELCAT_ATTR_RELNAME,relNameAttr,EQ);

    fprintf(logFile, "BlockAccess::renameAttribute::LinearSearch() search RELATIONCAT for wheter Relation Name exists? {%d,%d}\n",
    searchRelCatForRelName.block,searchRelCatForRelName.slot
    );
    fflush(logFile);

    if(searchRelCatForRelName.block==-1 && searchRelCatForRelName.slot==-1){
        return E_RELNOTEXIST;
    }



    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */

    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */



    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];



    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */


    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr

        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;

        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */

        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record

        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
        RecId attrcatRecId= linearSearch(ATTRCAT_RELID,ATTRCAT_ATTR_RELNAME,relNameAttr,EQ);

        fprintf(logFile, "BlockAccess::renameAttribute::LinearSearch() search ATTRIBUTECAT for next Relation Name location. It is -1 when no more attributes left {%d,%d}\n",
        attrcatRecId.block,attrcatRecId.slot
        );
        fflush(logFile);
        
      
        if(attrcatRecId.block==-1 && attrcatRecId.slot==-1){
            break;
        }
        RecBuffer attrcatentry(attrcatRecId.block);
        attrcatentry.getRecord(attrCatEntryRecord,attrcatRecId.slot);
        fprintf(logFile,"BlockAccess::renameAttribute: Initialize a Record Buffer with current RelName Block and load all its records");
        fflush(logFile);


        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,oldName)==0){
            attrToRenameRecId.block= attrcatRecId.block;
            attrToRenameRecId.slot=attrcatRecId.slot;
        }
        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName)==0){
            return E_ATTREXIST;
        }

    }


    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;


    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord


    if(attrToRenameRecId.block==-1 && attrToRenameRecId.slot==-1){
        return E_ATTRNOTEXIST;
    }

    RecBuffer attrToRenameRecBuffer(attrToRenameRecId.block);
    Attribute attrToRenameRecord[ATTRCAT_NO_ATTRS];

    attrToRenameRecBuffer.getRecord(attrToRenameRecord,attrToRenameRecId.slot);

    fprintf(logFile,"BlockAccess::renameAttribute: Initialize the target Record Buffer with current RelName Block and load all its records");
    fflush(logFile);

    strcpy(attrToRenameRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);
    attrToRenameRecBuffer.setRecord(attrToRenameRecord,attrToRenameRecId.slot);

    fprintf(logFile,"BlockAccess::renameAttribute: Update Attribute Name in the disk using setRecord Fn");
    fflush(logFile);

    return SUCCESS;
}

