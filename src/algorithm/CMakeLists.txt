
add_library( algorithm jointboost.cpp kmeans.cpp )
target_link_libraries( algorithm util ann ${QT_QTCORE_LIBRARY} ${TBB_LIBRARIES} )
