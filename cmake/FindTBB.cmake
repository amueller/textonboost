# This module defines
# TBB_INCLUDE_DIRS, where to find task_scheduler_init.h, etc.
# TBB_LIBRARIES, the libraries to link against to use TBB.
# TBB_FOUND, If false, don't try to use TBB.


#-- Look for include directory and set ${TBB_INCLUDE_DIR}
find_path(TBB_INCLUDE_DIR tbb/task_scheduler_init.h PATHS /usr/include /usr/local/include /opt/local/include )
mark_as_advanced(TBB_INCLUDE_DIR)

find_library(TBB_LIBRARY        tbb       /usr/lib /usr/local/lib /opt/local/lib)
find_library(TBB_MALLOC_LIBRARY tbbmalloc /usr/lib /usr/local/lib /opt/local/lib)
mark_as_advanced(TBB_LIBRARY TBB_MALLOC_LIBRARY)

if (TBB_INCLUDE_DIR)
    if (TBB_LIBRARY)
        set (TBB_FOUND "YES")
        set (TBB_LIBRARIES ${TBB_LIBRARY} ${TBB_MALLOC_LIBRARY} ${TBB_LIBRARIES})
        set (TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR} CACHE PATH "TBB include directory" FORCE)
        mark_as_advanced(TBB_INCLUDE_DIRS TBB_LIBRARIES)
        message(STATUS "Found Intel TBB")
    endif (TBB_LIBRARY)
endif (TBB_INCLUDE_DIR)

if (NOT TBB_FOUND)
    message("ERROR: Intel TBB NOT found!")
    if (TBB_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find TBB library.")
    endif (TBB_FIND_REQUIRED)
endif (NOT TBB_FOUND)

