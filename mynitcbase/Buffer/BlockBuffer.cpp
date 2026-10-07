#include "Logger/logger.h"
#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum = blockNum;
}

BlockBuffer::BlockBuffer(char blockType){
    /* allocate a block on the disk and a buffer in memory to hold the new block of
    given type using getFreeBlock function and get the return error codes if any.

    set the blockNum field of the object to that of the allocated block
    number if the method returned a valid block number,
    otherwise set the error code returned as the block number.

    (The caller must check if the constructor allocatted block successfully
    by checking the value of block number field.)
  */
  int bType;
  if(blockType == 'R') bType=REC;
  else bType=UNUSED_BLK;

  int blockNum = getFreeBlock(blockType);
  if(blockNum == E_DISKFULL){
    this->blockNum=E_DISKFULL;
    return;
  }
  
  int bufferNum = StaticBuffer::getFreeBuffer(blockNum);
  this->blockNum=blockNum;


}

RecBuffer::RecBuffer() : BlockBuffer('R'){}

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
  memcpy(&head->pblock, buffer + 4, 4);
  memcpy(&head->numEntries, buffer + 16     /* fill this */, 4);
  memcpy(&head->numAttrs, buffer +20       /* fill this */, 4);
  memcpy(&head->rblock, buffer + 12        /* fill this */, 4);
  memcpy(&head->lblock, buffer + 8       /* fill this */, 4);

  memcpy(&head->numSlots, bufferPtr + 24, 4);
  
  return SUCCESS;
}

int BlockBuffer::setHeader(struct HeadInfo *head){

    
    // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    // cast bufferPtr to type HeadInfo*
    
    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed, return the error code

    // return SUCCESS;

  unsigned char *bufferPtr;
  int ret =loadBlockAndGetBufferPtr(&bufferPtr);
  if(ret!=SUCCESS) return ret;

  struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;
  
  bufferHeader->blockType = head->blockType;
  bufferHeader->lblock = head->lblock;
  bufferHeader->numAttrs = head->numAttrs;
  bufferHeader->numEntries = head->numEntries;
  bufferHeader->numSlots = head->numSlots;
  bufferHeader->pblock = head->pblock;
  bufferHeader->rblock = head->rblock;

  ret =StaticBuffer::setDirtyBit(this->blockNum);
  if(ret!=SUCCESS) return ret;

  fprintf(logFile,"BlockBuffer::setHeader: Updated Header for <Block %d>\n",this->blockNum);
  fflush(logFile);

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
  memcpy(slotMap, slotMapInBuffer, slotCount);

  return SUCCESS;
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
/* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    // get the header of the block using the getHeader() function

    //int numSlots = the number of slots in the block //;

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`

    // update dirty bit using StaticBuffer::setDirtyBit
    // if setDirtyBit failed, return the value returned by the call

    // return SUCCESS

  
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
  memcpy(slotMapInBuffer,slotMap, slotCount);

  ret = StaticBuffer::setDirtyBit(this->blockNum);
  if(ret!=SUCCESS) return ret;

  fprintf(logFile, "RecBuffer::setSlotMap()  Copied SlotMap back to buffer and set the dirty bit for BlockNo:%d\n",this->blockNum);
  fflush(logFile);

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

int BlockBuffer::setBlockType(int blockType){

    /* 
    get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). 

    if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        return the value returned by the call.

    store the input block type in the first 4 bytes of the buffer.
    (hint: cast bufferPtr to int32_t* and then assign it)
    *((int32_t *)bufferPtr) = blockType;

    update the StaticBuffer::blockAllocMap entry corresponding to the
    object's block number to `blockType`.

    update dirty bit by calling StaticBuffer::setDirtyBit()
    if setDirtyBit() failed
        return the returned value from the call

    return SUCCESS
  */
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if(ret!=SUCCESS) return ret;

  *((int32_t *)bufferPtr) = blockType;

  (StaticBuffer::blockAllocMap)[this->blockNum] = blockType;

  ret = StaticBuffer::setDirtyBit(this->blockNum);
  if(ret!=SUCCESS) return ret;

  fprintf(logFile,"BlockBuffer::setBlockType: Set Block Type in BAM locally for <Block: %d>",this->blockNum);
  fflush(logFile);

  return SUCCESS;

}

int BlockBuffer::getFreeBlock(int blockType){

    /*iterate through the StaticBuffer::blockAllocMap and find the block number
    of a free block in the disk.

    if no block is free, return E_DISKFULL.

    set the object's blockNum to the block number of the free block.

    find a free buffer using StaticBuffer::getFreeBuffer() .

    initialize the header of the block passing a struct HeadInfo with values
    pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    to the setHeader() function.

    update the block type of the block to the input block type using setBlockType().

    return block number of the free block.
    */

    int freeBlock=-1;
    for(int block=0;block<DISK_BLOCKS;block++){
      if(StaticBuffer::blockAllocMap[block] == UNUSED_BLK){
        freeBlock = block;
        break;;
      }
    }
    if(freeBlock == -1) return E_DISKFULL;
    this->blockNum = freeBlock;

    int freeBuffer =StaticBuffer::getFreeBuffer(this->blockNum);
    HeadInfo header;
    header.blockType=blockType;
    header.pblock=-1;
    header.lblock=-1;
    header.rblock=-1;
    header.numAttrs=0;
    header.numEntries=0;;
    header.numSlots=0;

    int ret = setHeader(&header);
    if(ret!=SUCCESS) return ret;

    ret = setBlockType(blockType);

    return freeBlock;
}

int BlockBuffer::getBlockNum(){
    //return corresponding block number.
    return this->blockNum;
}


