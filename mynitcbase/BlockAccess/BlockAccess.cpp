#include "BlockAccess.h"
#include <cstring>
#include <iostream>

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
