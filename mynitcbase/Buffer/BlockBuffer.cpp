#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {  
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }

  
  unsigned char buffer[BLOCK_SIZE];
  // read the block at this.blockNum into the buffer
  Disk::readBlock(buffer,this->blockNum);

  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->numSlots, buffer + 24, 4);
  memcpy(&head->numEntries, buffer + 16     /* fill this */, 4);
  memcpy(&head->numAttrs, buffer +20       /* fill this */, 4);
  memcpy(&head->rblock, buffer + 12        /* fill this */, 4);
  memcpy(&head->lblock, buffer + 8       /* fill this */, 4);

  memcpy(&head->numSlots, bufferPtr + 24, 4);
  
  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  struct HeadInfo head;

  // get the header using this.getHeader() function
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }


    this->getHeader(&head);
    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;
  
  // read the block at this.blockNum into a buffer


  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
    int recordSize = attrCount * ATTR_SIZE;
    unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize* slotNum); 

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  // get the header of the block using getHeader() function
  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  ret =this->getHeader(&head); 
  if(ret!=SUCCESS) return ret;

  int slotCount = head.numSlots/* number of slots in block from header */;

  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
  for(int i=0;i<slotCount;i++){
    slotMap[i] = slotMapInBuffer[i];
  }

  return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;
   if(attrType==1)// 1 correspondes to string
      diff = strcmp(attr1.sVal, attr2.sVal);
  else
      diff = attr1.nVal - attr2.nVal;

  if(diff >0) return 1;
  else if(diff==0) return 0;
  else return -1; 
}

/* NOTE: This function will NOT check if the block has been initialised as a
   record or an index block. It will copy whatever content is there in that
   disk block to the buffer.
   Also ensure that all the methods accessing and updating the block's data
   should call the loadBlockAndGetBufferPtr() function before the access or
   update is done. This is because the block might not be present in the
   buffer due to LRU buffer replacement. So, it will need to be bought back
   to the buffer before any operations can be done.
 */
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char ** buffPtr) {
    /* check whether the block is already present in the buffer
       using StaticBuffer.getBufferNum() */


    // if present (!=E_BLOCKNOTINBUFFER),
        // set the timestamp of the corresponding buffer to 0 and increment the
        // timestamps of all other occupied buffers in BufferMetaInfo.

    // else
        // get a free buffer using StaticBuffer.getFreeBuffer()

        // if the call returns E_OUTOFBOUND, return E_OUTOFBOUND here as
        // the blockNum is invalid

        // Read the block into the free buffer using readBlock()

    // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr

    // return SUCCESS;

    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
    
    if(bufferNum!=E_BLOCKNOTINBUFFER){
      StaticBuffer::incrementTimeStamp();
      StaticBuffer::metainfo[bufferNum].timeStamp=0;      
    }
    else{
      int freeBufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
      if(freeBufferNum==E_OUTOFBOUND){
        return E_OUTOFBOUND;
      }
      Disk::readBlock(StaticBuffer::blocks[freeBufferNum],this->blockNum);
      bufferNum = freeBufferNum;
    }
    *buffPtr = StaticBuffer::blocks[bufferNum];

    return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    /* get the header of the block using the getHeader() function */

    // get number of attributes in the block.

    // get the number of slots in the block.

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */

    // update dirty bit using setDirtyBit()

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */

    // return SUCCESS

    unsigned char* bufferPtr;
    int ret =loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret!=SUCCESS){
      return ret; 
    }
    HeadInfo header;
    this->getHeader(&header);
    int numAttr = header.numAttrs;
    int numSlots = header.numSlots;

    if(slotNum<0 || slotNum>=numSlots){
      return E_OUTOFBOUND;
    }
    int recordSize = numAttr * ATTR_SIZE;
    unsigned char *slotptr = bufferPtr + HEADER_SIZE + numSlots + (slotNum * recordSize);

    memcpy(slotptr,rec,recordSize);
    StaticBuffer::setDirtyBit(this->blockNum);
    return SUCCESS;
}