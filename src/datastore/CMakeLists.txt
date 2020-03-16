
SET(DatastoreSRCS
        datastore.cc
)
SET(CouchBaseHelperSRCS
        couchbase_helper.cc
)

# shared library - libgvdsdatastore.so
ADD_LIBRARY(gvdsdatastore SHARED ${DatastoreSRCS} ${CouchBaseHelperSRCS})