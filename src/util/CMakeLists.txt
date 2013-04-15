
add_library( util colorconvertion.cpp util.cpp labelimage.cpp image.cpp colorimage.cpp segmentationimage.cpp )
target_link_libraries( util ${QT_QTGUI_LIBRARY} )
