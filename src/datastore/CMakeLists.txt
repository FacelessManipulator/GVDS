
SET(DatastoreSRCS
        datastore.cc
)
SET(CouchBaseHelperSRCS
        couchbase_helper.cc
)

# shared library - libhvsdatastore.so
ADD_LIBRARY(hvsdatastore SHARED ${DatastoreSRCS} ${CouchBaseHelperSRCS})