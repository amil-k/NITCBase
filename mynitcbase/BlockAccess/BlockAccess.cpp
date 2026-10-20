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

int BlockAccess::insert(int relId, Attribute *record) {
    // cout<<record[1].nVal<<endl;
    // get the relation catalog entry from relation cache
    // ( use RelCacheTable::getRelCatEntry() of Cache Layer)
    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(relId, &relCatEntry);

    /* first record block of the relation (from the rel-cat entry)*/;
    int blockNum = relCatEntry.firstBlk;

    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relCatEntry.numSlotsPerBlk;
    /* number of slots per record block */
    int numOfAttributes = relCatEntry.numAttrs;
    /* number of attributes of the relation */

    int prevBlockNum = -1;
    // keeps track of prev element in linked list
    /* block number of the last element in the linked list = -1 */

    /*
        Traversing the linked list of existing record blocks of the relation
        until a free slot is found OR
        until the end of the list is reached
    */
    while (blockNum != -1) {
        // create a RecBuffer object for blockNum (using appropriate constructor!)
        RecBuffer recBuffer(blockNum);

        // get header of block(blockNum) using RecBuffer::getHeader() function
        HeadInfo headInfo;
        recBuffer.getHeader(&headInfo);
        
        // get slot map of block(blockNum) using RecBuffer::getSlotMap() function
        unsigned char slotMap[numOfSlots];
        recBuffer.getSlotMap(slotMap);

        // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
        // (Free slot can be found by iterating over the slot map of the block)
        /* slot map stores SLOT_UNOCCUPIED if slot is free and
           SLOT_OCCUPIED if slot is occupied) */
        for(int i=0;i<numOfSlots;i++)
            if(slotMap[i]==SLOT_UNOCCUPIED){
                rec_id.block=blockNum;
                rec_id.slot=i;
                break;
            }

        /* if a free slot is found, set rec_id and discontinue the traversal
           of the linked list of record blocks (break from the loop) */
        if(rec_id.slot!=-1)
            break;
        /* otherwise, continue to check the next block by updating the
           block numbers as follows:
              update prevBlockNum = blockNum
              update blockNum = header.rblock (next element in the linked
                                               list of record blocks)
        */
        prevBlockNum=blockNum;
        blockNum=headInfo.rblock;
    }

    //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
    if(rec_id.slot==-1){
        // if relation is RELCAT, do not allocate any more blocks
        //     return E_MAXRELATIONS;
        if(relId==RELCAT_RELID) 
            return E_MAXRELATIONS;
        // Otherwise,
        // get a new record block (using the appropriate RecBuffer constructor!)
        RecBuffer newRecBuffer;
        // get the block number of the newly allocated block
        int newBlock=newRecBuffer.getBlockNum();
        // (use BlockBuffer::getBlockNum() function)
        // let ret be the return value of getBlockNum() function call
        if (newBlock == E_DISKFULL) {
            return E_DISKFULL;
        }
        // Assign rec_id.block = new block number(i.e. ret) and rec_id.slot = 0
        rec_id.block=newBlock;
        rec_id.slot=0;

        /*
            set the header of the new record block such that it links with
            existing record blocks of the relation
            set the block's header as follows:
            blockType: REC, pblock: -1
            lblock
                  = -1 (if linked list of existing record blocks was empty
                         i.e this is the first insertion into the relation)
                  = prevBlockNum (otherwise),
            rblock: -1, numEntries: 0,
            numSlots: numOfSlots, numAttrs: numOfAttributes
            (use BlockBuffer::setHeader() function)
        */
        HeadInfo newHeader;
        newHeader.blockType=REC;
        newHeader.pblock=-1;
        newHeader.lblock=prevBlockNum; 
        // -1 if no blocks for this rel till now
        newHeader.rblock=-1;
        newHeader.numEntries=0;
        newHeader.numSlots=numOfSlots;
        newHeader.numAttrs=numOfAttributes;
        
        newRecBuffer.setHeader(&newHeader);
        /*
            set block's slot map with all slots marked as free
            (i.e. store SLOT_UNOCCUPIED for all the entries)
            (use RecBuffer::setSlotMap() function)
        */
        unsigned char newSlotMap[numOfSlots];
        for(int i=0;i<numOfSlots;i++)
            newSlotMap[i]=SLOT_UNOCCUPIED;
        newRecBuffer.setSlotMap(newSlotMap);

        // if prevBlockNum != -1
        if(prevBlockNum!=-1){
            // create a RecBuffer object for prevBlockNum
            RecBuffer prevRecBuffer(prevBlockNum);

            // get the header of the block prevBlockNum and
            HeadInfo prevHeader;
            prevRecBuffer.getHeader(&prevHeader);
            
            // update the rblock field of the header to the new block
            // number i.e. rec_id.block
            prevHeader.rblock=rec_id.block;
            // (use BlockBuffer::setHeader() function)
            prevRecBuffer.setHeader(&prevHeader);
        }
        else{
            // update first block field in the relation catalog entry to the
            relCatEntry.firstBlk=newBlock;
            // new block (using RelCacheTable::setRelCatEntry() function)
        }
        // update last block field in the relation catalog entry to the
        relCatEntry.lastBlk=newBlock;

        // new block (using RelCacheTable::setRelCatEntry() function)
        RelCacheTable::setRelCatEntry(relId, &relCatEntry);
    }

    // create a RecBuffer object for rec_id.block
    RecBuffer recBuffer(rec_id.block);

    // insert the record into rec_id'th slot using RecBuffer.setRecord())
    recBuffer.setRecord(record, rec_id.slot);

    /* update the slot map of the block by marking entry of the slot to
       which record was inserted as occupied) */
    // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
    // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)

    unsigned char slotMap[numOfSlots];
    recBuffer.getSlotMap(slotMap);
    slotMap[rec_id.slot]=SLOT_OCCUPIED;
    recBuffer.setSlotMap(slotMap);

    // increment the numEntries field in the header of the block to
    // which record was inserted
    // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
    HeadInfo headInfo;
    recBuffer.getHeader(&headInfo);
    headInfo.numEntries++;
    recBuffer.setHeader(&headInfo);
    // Increment the number of records field in the relation cache entry for
    // the relation. (use RelCacheTable::setRelCatEntry function)
    relCatEntry.numRecs++;
    RelCacheTable::setRelCatEntry(relId, &relCatEntry); 
    return SUCCESS;
}

/*
NOTE: This function will copy the result of the search to the `record` argument.
      The caller should ensure that space is allocated for `record` array
      based on the number of attributes in the relation.
*/

int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    // Declare a variable called recid to store the searched record

    /* search for the record id (recid) corresponding to the attribute with
    attribute name attrName, with value attrval and satisfying the condition op
    using linearSearch() */

    // if there's no record satisfying the given condition (recId = {-1, -1})
    //    return E_NOTFOUND;

    /* Copy the record with record id (recId) to the record buffer (record)
       For this Instantiate a RecBuffer class object using recId and
       call the appropriate method to fetch the record
    */
    RecId recId;
    recId = linearSearch(relId,attrName,attrVal,EQ);
    if(recId.block==-1 && recId.slot==-1) return E_NOTFOUND;

    RecBuffer recordBuffer(recId.block);
    int retVal =recordBuffer.getRecord(record,recId.slot);
    
    if(retVal!=SUCCESS) return retVal;

    return SUCCESS;
}

int BlockAccess::deleteRelation(char relName[ATTR_SIZE]) {
    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    // (stores relName as type union Attribute)
    // assign relNameAttr.sVal = relName

    //  linearSearch on the relation catalog for RelName = relNameAttr

    // if the relation does not exist (linearSearch returned {-1, -1})
    //     return E_RELNOTEXIST
    
    /* store the relation catalog record corresponding to the relation in
       relCatEntryRecord using RecBuffer.getRecord */


    if(strcmp(relName,RELCAT_RELNAME)==0 || strcmp(relName,ATTRCAT_RELNAME)==0)
        return E_NOTPERMITTED;
    int retVal = RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr; strcpy(relNameAttr.sVal,relName);
    RecId recId = linearSearch(RELCAT_RELID,RELCAT_ATTR_RELNAME,relNameAttr,EQ);
    if(recId.slot==-1 && recId.block==-1) return E_RELNOTEXIST;
    
    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];
    RecBuffer relCatEntryBlock(recId.block);
    retVal = relCatEntryBlock.getRecord(relCatEntryRecord,recId.slot);
    if(retVal!=SUCCESS) return retVal;

    /* get the first record block of the relation (firstBlock) using the
       relation catalog entry record */
    /* get the number of attributes corresponding to the relation (numAttrs)
       using the relation catalog entry record */

    /*
     Delete all the record blocks of the relation
    */
    // for each record block of the relation:
    //     get block header using BlockBuffer.getHeader
    //     get the next block from the header (rblock)
    //     release the block using BlockBuffer.releaseBlock
    //
    //     Hint: to know if we reached the end, check if nextBlock = -1


    /***
        Deleting attribute catalog entries corresponding the relation and index
        blocks corresponding to the relation with relName on its attributes
    ***/

    // reset the searchIndex of the attribute catalog


    int firstBlk = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;
    int numAttrs = relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    int currentBlkNum = firstBlk;
    while(currentBlkNum!=-1){
        RecBuffer currentBlkBuffer(currentBlkNum);
        HeadInfo currentBlkHeader;
        retVal =currentBlkBuffer.getHeader(&currentBlkHeader);
        if(retVal!=SUCCESS) return retVal;

        int nextBlkNum = currentBlkHeader.rblock;
        currentBlkBuffer.releaseBlock();

        currentBlkNum = nextBlkNum;
    }

    int numberOfAttributesDeleted = 0;
    retVal = RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    while(true) {
        // attrCatRecId = linearSearch on attribute catalog for RelName = relNameAttr
        // if no more attributes to iterate over (attrCatRecId == {-1, -1})
        //     break;
        
        // create a RecBuffer for attrCatRecId.block
        // get the header of the block
        // get the record corresponding to attrCatRecId.slot

        // declare variable rootBlock which will be used to store the root
        // block field from the attribute catalog record.
        
        RecId attrCatRecId = linearSearch(ATTRCAT_RELID,ATTRCAT_ATTR_RELNAME,relNameAttr,EQ);
        if(attrCatRecId.block==-1 && attrCatRecId.slot==-1) break;

        numberOfAttributesDeleted++;
        RecBuffer attrCatEntryBlock(attrCatRecId.block);
        HeadInfo attrCatHeader;
        retVal = attrCatEntryBlock.getHeader(&attrCatHeader);
        if(retVal!=SUCCESS) return retVal;

        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        retVal = attrCatEntryBlock.getRecord(attrCatRecord,attrCatRecId.slot);
        if(retVal!=SUCCESS) return retVal;

        int rootBlock = attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal;

        
        // (This will be used later to delete any indexes if it exists)

        // Update the Slotmap for the block by setting the slot as SLOT_UNOCCUPIED
        // Hint: use RecBuffer.getSlotMap and RecBuffer.setSlotMap

        /* Decrement the numEntries in the header of the block corresponding to
           the attribute catalog entry and then set back the header
           using RecBuffer.setHeader */

        /* If number of entries become 0, releaseBlock is called after fixing
           the linked list.
        */

        unsigned char slotMap[attrCatHeader.numSlots];
        retVal =attrCatEntryBlock.getSlotMap(slotMap);
        if(retVal!=SUCCESS) return retVal;

        slotMap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
        retVal =attrCatEntryBlock.setSlotMap(slotMap);
        if(retVal!=SUCCESS) return retVal;

        /*   header.numEntries == 0  */
         /* Standard Double Linked List Delete for a Block
               Get the header of the left block and set it's rblock to this
               block's rblock
            */

            // create a RecBuffer for lblock and call appropriate methods
        attrCatHeader.numEntries--;
        retVal =attrCatEntryBlock.setHeader(&attrCatHeader);

        if (attrCatHeader.numEntries==0) {
            
            RecBuffer attrCatLeftBlk(attrCatHeader.lblock);
            HeadInfo leftHeader;
            retVal = attrCatLeftBlk.getHeader(&leftHeader);
            if(retVal!=SUCCESS) return retVal;

            leftHeader.rblock = attrCatHeader.rblock;
            attrCatLeftBlk.setHeader(&leftHeader);

            /* header.rblock != -1 */
                /* Get the header of the right block and set it's lblock to
                 this block's lblock */
                // create a RecBuffer for rblock and call appropriate methods
            //else
                // (the block being released is the "Last Block" of the relation.)
                /* update the Relation Catalog entry's LastBlock field for this
                   relation with the block number of the previous block. */

            if (attrCatHeader.rblock!=INVALID_BLOCKNUM) {
                RecBuffer attrCatRigthBlk(attrCatHeader.rblock);
                HeadInfo rightHeader;
                retVal = attrCatRigthBlk.getHeader(&rightHeader);
                if(retVal!=SUCCESS) return retVal;

                rightHeader.lblock = attrCatHeader.lblock;
                retVal =attrCatRigthBlk.setHeader(&rightHeader);
                if (retVal != SUCCESS) return retVal;

            } else {
                RelCatEntry relCatEntryBuffer;
				RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);
				relCatEntryBuffer.lastBlk = attrCatHeader.lblock;
                retVal = RelCacheTable::setRelCatEntry(ATTRCAT_RELID,&relCatEntryBuffer);
                if (retVal != SUCCESS) return retVal;
            }

            // (Since the attribute catalog will never be empty(why?), we do not
            //  need to handle the case of the linked list becoming empty - i.e
            //  every block of the attribute catalog gets released.)

            // call releaseBlock()

            attrCatEntryBlock.releaseBlock();
        }

        // (the following part is only relevant once indexing has been implemented)
        // if index exists for the attribute (rootBlock != -1), call bplus destroy
        // if (rootBlock != -1) {
        //     // delete the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
        // }
    }

    /*** Delete the entry corresponding to the relation from relation catalog ***/
    // Fetch the header of Relcat block

    /* Decrement the numEntries in the header of the block corresponding to the
       relation catalog entry and set it back */

    /* Get the slotmap in relation catalog, update it by marking the slot as
       free(SLOT_UNOCCUPIED) and set it back. */

    /*** Updating the Relation Cache Table ***/
    /** Update relation catalog record entry (number of records in relation
        catalog is decreased by 1) **/
    // Get the entry corresponding to relation catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

    /** Update attribute catalog entry (number of records in attribute catalog
        is decreased by numberOfAttributesDeleted) **/
    // i.e., #Records = #Records - numberOfAttributesDeleted

    // Get the entry corresponding to attribute catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)



    HeadInfo relCatHeader;
    retVal = relCatEntryBlock.getHeader(&relCatHeader);
    if(retVal!=SUCCESS) return retVal;
    
    relCatHeader.numEntries--;
    retVal = relCatEntryBlock.setHeader(&relCatHeader);
    if(retVal!=SUCCESS) return retVal;
    

    unsigned char slotMap[relCatHeader.numSlots];
    relCatEntryBlock.getSlotMap(slotMap);

    slotMap[recId.slot] = SLOT_UNOCCUPIED;
    relCatEntryBlock.setSlotMap(slotMap);

    RelCatEntry relCatEntryBuffer;
	RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatEntryBuffer);
    relCatEntryBuffer.numRecs--;
    RelCacheTable::setRelCatEntry(RELCAT_RELID, &relCatEntryBuffer);
    
    RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);
	relCatEntryBuffer.numRecs -= numberOfAttributesDeleted;
	RelCacheTable::setRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);


    return SUCCESS;
}

/*
NOTE: the caller is expected to allocate space for the argument `record` based
      on the size of the relation. This function will only copy the result of
      the projection onto the array pointed to by the argument.
*/
int BlockAccess::project(int relId, Attribute *record) {
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)

    // declare block and slot which will be used to store the record id of the
    // slot we need to check.

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */

    int block, slot;
    RecId prevRecID;
    RelCacheTable::getSearchIndex(relId,&prevRecID);
    RelCatEntry relCatEntry;
    int retVal =RelCacheTable::getRelCatEntry(relId,&relCatEntry);
    if(retVal!=SUCCESS)return retVal;


    if (prevRecID.block == -1 && prevRecID.slot == -1)
    {
        // (new project operation. start from beginning)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)

        // block = first record block of the relation
        // slot = 0


        block = relCatEntry.firstBlk;
        slot = 0;
    }
    else
    {
        // (a project/search operation is already in progress)
        // block = previous search index's block
        // slot = previous search index's slot + 1

        block = prevRecID.block;
        slot = prevRecID.slot+1;

    }


    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */
    while (block != -1)
    {
        // create a RecBuffer object for block (using appropriate constructor!)

        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function

        RecBuffer recBuffer(block);
        unsigned char slotMap[relCatEntry.numSlotsPerBlk];
        retVal = recBuffer.getSlotMap(slotMap);
        if(retVal!=SUCCESS)return retVal;


        if(/* slot >= the number of slots per block*/ slot>=relCatEntry.numSlotsPerBlk)
        {
            // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            // (NOTE: if this is the last block, rblock would be -1. this would
            //        set block = -1 and fail the loop condition )
            HeadInfo header;
            recBuffer.getHeader(&header);
            block = header.rblock;
            slot = 0;
        }
        else if (/* slot is free */ slotMap[slot]==SLOT_UNOCCUPIED)
        { // (i.e slot-th entry in slotMap contains SLOT_UNOCCUPIED)
            // increment slot
            slot = slot+1;
        }
        else {
            // (the next occupied slot / record has been found)
            break;
        }

    }

    if (block == -1){
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found

    // set the search index to nextRecId using RelCacheTable::setSearchIndex

    /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */

    RecId nextRecId = {block, slot};
    retVal = RelCacheTable::setSearchIndex(relId,&nextRecId);
    if(retVal!=SUCCESS)return retVal;

    RecBuffer recBuffer(block);
    retVal = recBuffer.getRecord(record,slot);
    if(retVal!=SUCCESS)return retVal;

    return SUCCESS;
}

