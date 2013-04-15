
add_library( feature bboxfeature.cpp hogfeature.cpp locationfeature.cpp colorfeature.cpp feature.cpp filterbank.cpp texton.cpp )
target_link_libraries( feature algorithm util ${QT_QTCORE_LIBRARY} )
