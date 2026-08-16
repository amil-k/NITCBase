#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <cstring>
using namespace std;


int main(int argc, char *argv[]) {
  Disk disk_run;

  StaticBuffer buffer;
  OpenRelTable cache;

  /*
  for i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)

      get the relation catalog entry using RelCacheTable::getRelCatEntry()
      printf("Relation: %s\n", relname);

      for j = 0 to numAttrs of the relation - 1
          get the attribute catalog entry for (rel-id i, attribute offset j)
           in attrCatEntry using AttrCacheTable::getAttrCatEntry()

          printf("  %s: %s\n", attrName, attrType);
  */

  for(int i=0;i<=2;i++){
    RelCatEntry relCatBuffer;

    int RelCatEntry = RelCacheTable::getRelCatEntry(i,&relCatBuffer);

    printf("Relation: %s\n", relCatBuffer.relName);

    for(int j=0;j<relCatBuffer.numAttrs;j++){
        AttrCatEntry AttrCatBuffer;
        AttrCacheTable::getAttrCatEntry(i,j,&AttrCatBuffer);
        const char* attrType = AttrCatBuffer.attrType==NUMBER ? "NUM":"STR";
        printf("  %s: %s\n", AttrCatBuffer.attrName, attrType);
    }
    

  }

  return 0;
}