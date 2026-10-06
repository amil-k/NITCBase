#include "StaticBuffer.h"
// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {

  // initialise all blocks as free

  //for (/*bufferIndex = 0 to BUFFER_CAPACITY-1*/) {
    // set metainfo[bufferindex] with the following values
    //   free = true
    //   dirty = false
    //   timestamp = -1
    //   blockNum = -1
  //}


  for (int bufferIndex =0;bufferIndex<BUFFER_CAPACITY;bufferIndex++ /*bufferIndex = 0 to BUFFER_CAPACITY-1*/) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty = false;
    metainfo[bufferIndex].timeStamp = -1;
    metainfo[bufferIndex].blockNum = -1;
    
  }
// write back all modified blocks on system exit
}


StaticBuffer::~StaticBuffer() {

  /*iterate through all the buffer blocks,
    write back blocks with metainfo as free=false,dirty=true
    using Disk::writeBlock()
    */
  for (int bufferIndex =0;bufferIndex<BUFFER_CAPACITY;bufferIndex++ /*bufferIndex = 0 to BUFFER_CAPACITY-1*/) {
    if(metainfo[bufferIndex].free==false && metainfo[bufferIndex].dirty==true){
      Disk::writeBlock(blocks[bufferIndex],metainfo[bufferIndex].blockNum);
    }
  }
  

}

int StaticBuffer::getFreeBuffer(int blockNum) {
  if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
  // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.

  // increase the timeStamp in metaInfo of all occupied buffers.

  // let bufferNum be used to store the buffer number of the free/freed buffer.

  // iterate through metainfo and check if there is any buffer free

  // if a free buffer is available, set bufferNum = index of that free buffer.

  // if a free buffer is not available,
  //     find the buffer with the largest timestamp
  //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
  //     set bufferNum = index of this buffer

  // update the metaInfo entry corresponding to bufferNum with
  // free:false, dirty:false, blockNum:the input block number, timeStamp:0.

  // return the bufferNum.
  int allocatedBuffer=-1;
  int largestTimeStamp=-1;
  int bufferWithLargestTimeStamp=-1;


  for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++){
    if(metainfo[bufferIndex].free==true){
      allocatedBuffer = bufferIndex;
      break;
    }

    metainfo[bufferIndex].timeStamp++;
    if(metainfo[bufferIndex].timeStamp > largestTimeStamp){
      largestTimeStamp = metainfo[bufferIndex].timeStamp;
      bufferWithLargestTimeStamp = bufferIndex;
    }
  
  }


  if(allocatedBuffer ==-1){
    if(metainfo[bufferWithLargestTimeStamp].dirty==true){
      Disk::writeBlock(blocks[bufferWithLargestTimeStamp],metainfo[bufferWithLargestTimeStamp].blockNum);
    }
    allocatedBuffer = bufferWithLargestTimeStamp;
  }

  metainfo[allocatedBuffer].free = false;
  metainfo[allocatedBuffer].dirty = false; 
  metainfo[allocatedBuffer].blockNum = blockNum;
  metainfo[allocatedBuffer].timeStamp=0;

  return allocatedBuffer;
}

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {
  // Check if blockNum is valid (between zero and DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.
  if(blockNum<0 || blockNum >=DISK_BLOCKS ) return E_OUTOFBOUND;

  // find and return the bufferIndex which corresponds to blockNum (check metainfo)
  for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++){
    if(metainfo[bufferIndex].blockNum==blockNum)
      return bufferIndex;
  }

  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().

    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER

    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND

    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo

    // return SUCCESS


  int bufferNum = getBufferNum(blockNum);
  
  if(bufferNum == E_BLOCKNOTINBUFFER){
    return E_BLOCKNOTINBUFFER;
  }
  if(blockNum <0 || blockNum >=DISK_BLOCKS){
    return E_OUTOFBOUND;
  }

  metainfo[bufferNum].dirty=true;
  return SUCCESS;
}

//added
int StaticBuffer::incrementTimeStamp(){
  for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++){
    
    if(metainfo[bufferIndex].free==false){
      metainfo[bufferIndex].timeStamp++;
    }
  }
  return SUCCESS;
};