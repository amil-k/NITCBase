#include "RelCacheTable.h"
#include "Logger/logger.h"
#include <cstring>

RelCacheEntry* RelCacheTable::relCache[MAX_OPEN];

/*
Get the relation catalog entry for the relation with rel-id `relId` from the cache
NOTE: this function expects the caller to allocate memory for `*relCatBuf`
Final State Achieved in Stage 3
*/

int RelCacheTable::getRelCatEntry(int relId, RelCatEntry* relCatBuf) {
    fprintf(logFile,
            "RelCacheTable.cpp::RelCacheTable::getRelCatEntry(relId=%d, relCatBuf=%p):: "
            "getting the relation catalog entry from the relation cache\n",
            relId, (void *)relCatBuf);
    fflush(logFile);

    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    // if there's no entry at the rel-id
    if (relCache[relId] == nullptr) {
        fprintf(logFile,
                "RelCacheTable.cpp::RelCacheTable::getRelCatEntry(relId=%d, relCatBuf=%p):: "
                "no cache entry at this relId, returning E_RELNOTOPEN\n",
                relId, (void *)relCatBuf);
        fflush(logFile);
        return E_RELNOTOPEN;
    }

    *relCatBuf = relCache[relId]->relCatEntry;

    fprintf(logFile,
            "RelCacheTable.cpp::RelCacheTable::getRelCatEntry(relId=%d, relCatBuf=%p):: "
            "copied the relation catalog entry, returning SUCCESS\n",
            relId, (void *)relCatBuf);
    fflush(logFile);
    return SUCCESS;
}

/* Converts a relation catalog record to RelCatEntry struct
We get the record as Attribute[] from the BlockBuffer.getRecord() function.
This function will convert that to a struct RelCatEntry type.
NOTE: this function expects the caller to allocate memory for `*relCatEntry`
Final State Achieved in Stage 3
*/

void RelCacheTable::recordToRelCatEntry(union Attribute record[RELCAT_NO_ATTRS], RelCatEntry* relCatEntry) {
    strcpy(relCatEntry->relName, record[RELCAT_REL_NAME_INDEX].sVal);
    relCatEntry->numAttrs = (int)record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    relCatEntry->numRecs = (int)record[RELCAT_NO_RECORDS_INDEX].nVal;
    relCatEntry->firstBlk = (int)record[RELCAT_FIRST_BLOCK_INDEX].nVal;
    relCatEntry->lastBlk = (int)record[RELCAT_LAST_BLOCK_INDEX].nVal;
    relCatEntry->numSlotsPerBlk = (int)record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal;
}

void RelCacheTable::relCatEntryToRecord(RelCatEntry *relCatEntry, Attribute record[RELCAT_NO_ATTRS]) {
    strcpy(record[RELCAT_REL_NAME_INDEX].sVal, relCatEntry->relName);
    record[RELCAT_NO_ATTRIBUTES_INDEX].nVal = relCatEntry->numAttrs;
    record[RELCAT_NO_RECORDS_INDEX].nVal = relCatEntry->numRecs;
    record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = relCatEntry->numSlotsPerBlk;
    record[RELCAT_FIRST_BLOCK_INDEX].nVal = relCatEntry->firstBlk;
    record[RELCAT_LAST_BLOCK_INDEX].nVal = relCatEntry->lastBlk;
}

/* will return the searchIndex for the relation corresponding to `relId
NOTE: this function expects the caller to allocate memory for `*searchIndex`
Final State Achieved in Stage 4
*/
int RelCacheTable::getSearchIndex(int relId, RecId* searchIndex) {
    fprintf(logFile,
            "RelCacheTable.cpp::RelCacheTable::getSearchIndex(relId=%d, searchIndex=%p):: "
            "getting the search index of the relation from the relation cache\n",
            relId, (void *)searchIndex);
    fflush(logFile);

    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    // if there's no entry at the rel-id
    if (relCache[relId] == nullptr) {
        return E_RELNOTOPEN;
    }

    *searchIndex = relCache[relId]->searchIndex;

    fprintf(logFile,
            "RelCacheTable.cpp::RelCacheTable::getSearchIndex(relId=%d, searchIndex=%p):: "
            "search index is (block=%d, slot=%d), returning SUCCESS\n",
            relId, (void *)searchIndex, searchIndex->block, searchIndex->slot);
    fflush(logFile);
    return SUCCESS;
}

int RelCacheTable::setSearchIndex(int relId, RecId* searchIndex) {
    fprintf(logFile,
            "RelCacheTable.cpp::RelCacheTable::setSearchIndex(relId=%d, searchIndex=%p):: "
            "setting the search index of the relation in the relation cache\n",
            relId, (void *)searchIndex);
    fflush(logFile);

    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    // if there's no entry at the rel-id
    if (relCache[relId] == nullptr) {
        return E_RELNOTOPEN;
    }

    relCache[relId]->searchIndex = *searchIndex;

    fprintf(logFile,
            "RelCacheTable.cpp::RelCacheTable::setSearchIndex(relId=%d, searchIndex=%p):: "
            "search index set to (block=%d, slot=%d), returning SUCCESS\n",
            relId, (void *)searchIndex, searchIndex->block, searchIndex->slot);
    fflush(logFile);
    return SUCCESS;
}

int RelCacheTable::resetSearchIndex(int relId) {
    RecId initVal = {-1, -1};
    return setSearchIndex(relId, &initVal);
}

int RelCacheTable::setRelCatEntry(int relId, RelCatEntry *relCatBuf) {
    fprintf(logFile,
            "RelCacheTable.cpp::RelCacheTable::setRelCatEntry(relId=%d, relCatBuf=%p):: "
            "writing the relation catalog entry into the relation cache\n",
            relId, (void *)relCatBuf);
    fflush(logFile);

    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    // if there's no entry at the rel-id
    if (relCache[relId] == nullptr) {
        return E_RELNOTOPEN;
    }

    // copy the relCatBuf to the corresponding Relation Catalog entry in
    // the Relation Cache Table.
    relCache[relId]->relCatEntry = *relCatBuf;

    // set the dirty flag of the corresponding Relation Cache entry in
    // the Relation Cache Table.
    relCache[relId]->dirty = true;

    fprintf(logFile,
            "RelCacheTable.cpp::RelCacheTable::setRelCatEntry(relId=%d, relCatBuf=%p):: "
            "entry copied and dirty flag set, returning SUCCESS\n",
            relId, (void *)relCatBuf);
    fflush(logFile);
    return SUCCESS;
}

