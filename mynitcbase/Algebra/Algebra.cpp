#include "Algebra.h"
#include <cstring>
#include <iostream>
using namespace std;

/* used to select all the records that satisfy a condition.
the arguments of the function are
- srcRel - the source relation we want to select from
- targetRel - the relation we want to select into. (ignore for now)
- attr - the attribute that the condition is checking
- op - the operator of the condition
- strVal - the value that we want to compare against (represented as a string)

*/

bool isNumber(char *str) {
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's
    other characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]){
    
    if(strcmp(relName,RELCAT_RELNAME)==0 or strcmp(relName,ATTRCAT_RELNAME)==0){
      return E_NOTPERMITTED;
    }

    // get the relation's rel-id using OpenRelTable::getRelId() method
    int relId = OpenRelTable::getRelId(relName);
    
    if(relId==E_RELNOTOPEN){
      return E_RELNOTOPEN;
    }

    RelCatEntry relCatBuf;
    RelCacheTable::getRelCatEntry(relId,&relCatBuf);

    // if relation is not open in open relation table, return E_RELNOTOPEN
    // (check if the value returned from getRelId function call = E_RELNOTOPEN)
    // get the relation catalog entry from relation cache
    // (use RelCacheTable::getRelCatEntry() of Cache Layer)

    /* if relCatEntry.numAttrs != numberOfAttributes in relation,
       return E_NATTRMISMATCH */
       if(relCatBuf.numAttrs!=nAttrs){
        return E_NATTRMISMATCH;
       }

    // let recordValues[numberOfAttributes] be an array of type union Attribute
    Attribute recordValues[nAttrs];

    /*
        Converting 2D char array of record values to Attribute array recordValues
     */
    // iterate through 0 to nAttrs-1: (let i be the iterator)
    for(int i=0;i<nAttrs;i++)
    {
        // get the attr-cat entry for the i'th attribute from the attr-cache
        // (use AttrCacheTable::getAttrCatEntry())
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId,i,&attrCatEntry);
        int type=attrCatEntry.attrType;
        // let type = attrCatEntry.attrType;

        if (type == NUMBER)
        {
            // if the char array record[i] can be converted to a number
            // (check this using isNumber() function)
            if(isNumber(record[i]))
            {
                /* convert the char array to numeral and store it
                   at recordValues[i].nVal using atof() */
                   recordValues[i].nVal=atof(record[i]);
            }
            else
            {
                return E_ATTRTYPEMISMATCH;
            }
        }
        else if (type == STRING)
        {
            // copy record[i] to recordValues[i].sVal
            strcpy(recordValues[i].sVal,record[i]);
        }
    }
    int retVal=BlockAccess::insert(relId,recordValues);
    // insert the record by calling BlockAccess::insert() function
    // let retVal denote the return value of insert call

    return retVal;
}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {
    // get the srcRel's rel-id (let it be srcRelid), using OpenRelTable::getRelId()
    // if srcRel is not open in open relation table, return E_RELNOTOPEN

    // get the attr-cat entry for attr, using AttrCacheTable::getAttrCatEntry()
    // if getAttrcatEntry() call fails return E_ATTRNOTEXIST


    /*** Convert strVal to an attribute of data type NUMBER or STRING ***/

    int srcRelId = OpenRelTable::getRelId(srcRel);
    if(srcRelId==E_RELNOTOPEN) return E_RELNOTOPEN;

    AttrCatEntry attrCatEntry;
    int retVal = AttrCacheTable::getAttrCatEntry(srcRelId,attr,&attrCatEntry);
    if(retVal!=SUCCESS)return retVal;

    Attribute attrVal;
    int type = attrCatEntry.attrType;

    if (type == NUMBER)
    {
        // if the input argument strVal can be converted to a number
        // (check this using isNumber() function)
        if(isNumber(strVal))
        {
            // convert strVal to double and store it at attrVal.nVal using atof()
            attrVal.nVal = atof(strVal);
        }
        else
        {
            return E_ATTRTYPEMISMATCH;
        }
    }
    else if (type == STRING)
    {
        // copy strVal to attrVal.sVal
        strcpy(attrVal.sVal,strVal);
    }

    /*** Creating and opening the target relation ***/
    // Prepare arguments for createRel() in the following way:
    // get RelcatEntry of srcRel using RelCacheTable::getRelCatEntry()
    //int src_nAttrs = /* the no. of attributes present in src relation */ ;
    

    /* let attr_names[src_nAttrs][ATTR_SIZE] be a 2D array of type char
        (will store the attribute names of rel). */
    // let attr_types[src_nAttrs] be an array of type int

    /*iterate through 0 to src_nAttrs-1 :
        get the i'th attribute's AttrCatEntry using AttrCacheTable::getAttrCatEntry()
        fill the attr_names, attr_types arrays that we declared with the entries
        of corresponding attributes
    */
    RelCatEntry relCatEntry;
    retVal = RelCacheTable::getRelCatEntry(srcRelId,&relCatEntry);
    if(retVal!=SUCCESS)return retVal;
    int src_nAttrs= relCatEntry.numAttrs;

    char attr_name[src_nAttrs][ATTR_SIZE];
    int attr_types[src_nAttrs];

    for(int i=0;i<src_nAttrs;i++){
      AttrCatEntry tempAttrCatEntry;
      AttrCacheTable::getAttrCatEntry(srcRelId,i,&tempAttrCatEntry);
      strcpy(attr_name[i],tempAttrCatEntry.attrName);
      attr_types[i] = tempAttrCatEntry.attrType;
    }

    /* Create the relation for target relation by calling Schema::createRel()
       by providing appropriate arguments */
    // if the createRel returns an error code, then return that value.

    /* Open the newly created target relation by calling OpenRelTable::openRel()
       method and store the target relid */
    /* If opening fails, delete the target relation by calling Schema::deleteRel()
       and return the error value returned from openRel() */

    /*** Selecting and inserting records into the target relation ***/
    /* Before calling the search function, reset the search to start from the
       first using RelCacheTable::resetSearchIndex() */

    retVal = Schema::createRel(targetRel,src_nAttrs,attr_name,attr_types);
    if(retVal!=SUCCESS) return retVal;

    int targetRelId = OpenRelTable::openRel(targetRel);
    if(targetRelId <0 || targetRelId>=MAX_OPEN){
      retVal = Schema::deleteRel(targetRel);
      return targetRelId;
    }

    retVal = RelCacheTable::resetSearchIndex(targetRelId);
    Attribute record[src_nAttrs];

    /*
        The BlockAccess::search() function can either do a linearSearch or
        a B+ tree search. Hence, reset the search index of the relation in the
        relation cache using RelCacheTable::resetSearchIndex().
        Also, reset the search index in the attribute cache for the select
        condition attribute with name given by the argument `attr`. Use
        AttrCacheTable::resetSearchIndex().
        Both these calls are necessary to ensure that search begins from the
        first record.
    */
    RelCacheTable::resetSearchIndex(srcRelId);
    //AttrCacheTable::resetSearchIndex(srcRelId,attr);

    // read every record that satisfies the condition by repeatedly calling
    // BlockAccess::search() until there are no more records to be read
    /* BlockAccess::search() returns success */
    while (BlockAccess::search(srcRelId,record,attr,attrVal,op)==SUCCESS) {
        // ret = BlockAccess::insert(targetRelId, record);
        // if (insert fails) {
        //     close the targetrel(by calling Schema::closeRel(targetrel))
        //     delete targetrel (by calling Schema::deleteRel(targetrel))
        //     return ret;
        // }
        retVal = BlockAccess::insert(targetRelId,record);
        if(retVal!=SUCCESS){
          Schema::closeRel(targetRel);
          Schema::deleteRel(targetRel);
          return retVal;
        }

    }

    // Close the targetRel by calling closeRel() method of schema layer
    // return SUCCESS.

    Schema::closeRel(targetRel);
    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]) {

        
    
    /*srcRel's rel-id (use OpenRelTable::getRelId() function)*/

    // if srcRel is not open in open relation table, return E_RELNOTOPEN

    // get RelCatEntry of srcRel using RelCacheTable::getRelCatEntry()

    // get the no. of attributes present in relation from the fetched RelCatEntry.

    // attrNames and attrTypes will be used to store the attribute names
    // and types of the source relation respectively

    /*iterate through every attribute of the source relation :
    - get the AttributeCat entry of the attribute with offset.
        (using AttrCacheTable::getAttrCatEntry())
    - fill the arrays `attrNames` and `attrTypes` that we declared earlier
        with the data about each attribute
*/

    int srcRelId = OpenRelTable::getRelId(srcRel);
    if(srcRelId==E_RELNOTOPEN) return E_RELNOTOPEN;
    RelCatEntry relCatEntry;
    int retVal = RelCacheTable::getRelCatEntry(srcRelId,&relCatEntry);

    int numAttrs = relCatEntry.numAttrs;
    char attrNames[numAttrs][ATTR_SIZE];
    int attrTypes[numAttrs];

    for(int attr = 0; attr<numAttrs;attr++){
        AttrCatEntry attrCatEntry;
        retVal = AttrCacheTable::getAttrCatEntry(srcRelId,attr,&attrCatEntry);
        strcpy(attrNames[attr],attrCatEntry.attrName);
        attrTypes[attr]= attrCatEntry.attrType;
    }



    /*** Creating and opening the target relation ***/

    // Create a relation for target relation by calling Schema::createRel()

    // if the createRel returns an error code, then return that value.

    // Open the newly created target relation by calling OpenRelTable::openRel()
    // and get the target relid

    // If opening fails, delete the target relation by calling Schema::deleteRel() of
    // return the error value returned from openRel().


    /*** Inserting projected records into the target relation ***/

    // Take care to reset the searchIndex before calling the project function
    // using RelCacheTable::resetSearchIndex()


    retVal = Schema::createRel(targetRel,numAttrs,attrNames,attrTypes);
    if(retVal!=SUCCESS) return retVal;

    int targetRelId = OpenRelTable::openRel(targetRel);
    if(targetRelId<0 || targetRelId>=MAX_OPEN) {
        retVal = Schema::deleteRel(targetRel);
        if(retVal!=SUCCESS) return retVal;
        return targetRelId;
    }
    RelCacheTable::resetSearchIndex(srcRelId);
    Attribute record[numAttrs];


    while ( BlockAccess::project(srcRelId,record)==SUCCESS   /* BlockAccess::project(srcRelId, record) returns SUCCESS */){
        // record will contain the next record

        // ret = BlockAccess::insert(targetRelId, proj_record);

    //     if (/* insert fails */) {
    //         // close the targetrel by calling Schema::closeRel()
    //         // delete targetrel by calling Schema::deleteRel()
    //         // return ret;
    //     }
    // }

    // Close the targetRel by calling Schema::closeRel()

    // return SUCCESS.


        retVal = BlockAccess::insert(targetRelId,record);
        if(retVal!=SUCCESS){
            retVal=Schema::closeRel(targetRel);
            if(retVal!=SUCCESS) return retVal;
            retVal =Schema::deleteRel(targetRel);
            if(retVal!=SUCCESS) return retVal;
        }
    }
    retVal = Schema::closeRel(targetRel);
    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]) {

    //int srcRelId = /*srcRel's rel-id (use OpenRelTable::getRelId() function)*/

    // if srcRel is not open in open relation table, return E_RELNOTOPEN

    // get RelCatEntry of srcRel using RelCacheTable::getRelCatEntry()

    // get the no. of attributes present in relation from the fetched RelCatEntry.

    // declare attr_offset[tar_nAttrs] an array of type int.
    // where i-th entry will store the offset in a record of srcRel for the
    // i-th attribute in the target relation.

    // let attr_types[tar_nAttrs] be an array of type int.
    // where i-th entry will store the type of the i-th attribute in the
    // target relation.


    /*** Checking if attributes of target are present in the source relation
         and storing its offsets and types ***/

    /*iterate through 0 to tar_nAttrs-1 :
        - get the attribute catalog entry of the attribute with name tar_attrs[i].
        - if the attribute is not found return E_ATTRNOTEXIST
        - fill the attr_offset, attr_types arrays of target relation from the
          corresponding attribute catalog entries of source relation
    */


    int srcRelId = OpenRelTable::getRelId(srcRel);
    if(srcRelId==E_RELNOTOPEN) return E_RELNOTOPEN;
    RelCatEntry relCatEntry;
    int retVal = RelCacheTable::getRelCatEntry(srcRelId,&relCatEntry);

    int src_nAttrs = relCatEntry.numAttrs;
    int attr_offset[tar_nAttrs];
    int attr_Types[tar_nAttrs];


    for(int attr = 0; attr<tar_nAttrs;attr++){
        AttrCatEntry attrCatEntry;
        retVal = AttrCacheTable::getAttrCatEntry(srcRelId,tar_Attrs[attr],&attrCatEntry);
        if(retVal!=SUCCESS) return retVal;
        attr_offset[attr] = attrCatEntry.offset;
        attr_Types[attr]= attrCatEntry.attrType;
    }



    /*** Creating and opening the target relation ***/

    // Create a relation for target relation by calling Schema::createRel()

    // if the createRel returns an error code, then return that value.

    // Open the newly created target relation by calling OpenRelTable::openRel()
    // and get the target relid

    // If opening fails, delete the target relation by calling Schema::deleteRel()
    // and return the error value from openRel()


    /*** Inserting projected records into the target relation ***/

    // Take care to reset the searchIndex before calling the project function
    // using RelCacheTable::resetSearchIndex()


    retVal = Schema::createRel(targetRel,tar_nAttrs,tar_Attrs,attr_Types);
    if(retVal!=SUCCESS) return retVal;
    
    int targetRelId = OpenRelTable::openRel(targetRel);
    if(targetRelId!=SUCCESS){
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    RelCacheTable::resetSearchIndex(srcRelId);
    Attribute record[src_nAttrs];
        
    while (  BlockAccess::project(srcRelId,record)==SUCCESS  /* BlockAccess::project(srcRelId, record) returns SUCCESS */) {
        // the variable `record` will contain the next record
        //iterate through 0 to tar_attrs-1:
        //    proj_record[attr_iter] = record[attr_offset[attr_iter]]

        // ret = BlockAccess::insert(targetRelId, proj_record);

        // if (/* insert fails */) {
        //     // close the targetrel by calling Schema::closeRel()
        //     // delete targetrel by calling Schema::deleteRel()
        //     // return ret;
        // }
        Attribute proj_record[tar_nAttrs];
        for(int i=0;i<tar_nAttrs;i++){
            proj_record[i] = record[attr_offset[i]];
        }
        retVal = BlockAccess::insert(targetRelId,proj_record);
        if(retVal!=SUCCESS){
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return retVal;
        }
    }

    // Close the targetRel by calling Schema::closeRel()
    Schema::closeRel(targetRel);
    return SUCCESS;
    // return SUCCESS.
}