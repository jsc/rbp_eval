#ifndef RBPCC_DOCOCCUR_H
#define RBPCC_DOCOCCUR_H

/**
 *  Record document occurences across a set of systems.
 *
 *  Considering only a single topic.  Within this topic,
 *  we want to record what documents have occurred.  Then
 *  for each document, we want to record what runs that
 *  document has occurred in, and at what rank.
 *
 *  Having stored this information, we want to be able to
 *  search it by document and also by run.
 *
 *  Documents are identified by their docids, which are
 *  strings.  Runs are identified by the id of the system
 *  they belong to.  The topic is identified in the structure
 *  for each topic.
 *
 *  We assume that a document does not occur more than once
 *  in each run, and do not explicitly check for this case
 *  during insertion.
 *
 *  The information for the test set is just a list of 
 *  per-topic document occurence structures; we do not
 *  aggregate document occurence statistics across multiple
 *  topics.
 *
 *  The primary lookup table in the DocOccur structure is
 *  by document id.  This maps to an array of document
 *  occurence info.  During construction, new document
 *  occurence bits are simply appended to this array.
 *
 *  The DocOccur structure also has a secondary lookup table
 *  by system.  This points to a linked list of occurences
 *  for that system; the links link up the slots in the
 *  document arrays which the system has entries for.
 *  New entries are added to the head of this list.
 *
 *  So the process of adding a new entry is as follows:
 *
 *  We already know what system we are working with, and
 *  have retained a link to the head of this system's
 *  list of entries.
 *
 *  We look up the document id of the new entry, and
 *  retrieve the entry list for that document id.  The new
 *  entry is appended to the end of this list, and the newly
 *  created entry is added to the head of the system's linked
 *  list, along with the rank of the occurence.
 *
 *  XXX a probably better design is simply to have a
 *  two-dimensional array of system by document.  There will
 *  only be a few thousand distinct documents for each topic,
 *  and a hundred or so systems, so the array will have perhaps
 *  half a million elements.
 *
 *  In order to minimise dependencies with other modules,
 *  we perform our own management of document and system
 *  identifiers.  There are atomisers for each of these
 *  held in the testset object.  Thus, a unique, sequential,
 *  numeric id is assigned to each document and system identifier
 *  as they are added to the testset dococcurence structure.
 *  Within the topic structures, documents and systems are
 *  referred to solely by these ids.
 */

/*
 *  Information to store on a document occurence.
 *
 *  We want to remember the system the document occurred in, the
 *  rank within that system.
 */
class DocOccurInfo {
    private:
        std::string 
};

/*
 *  Records document occurences for a particular topic.
 */
class DocOccurTopic 

#endif /* RBPCC_DOCOCCUR_H */
