
qt5_wrap_cpp(MOC 
    ${PROJECT_ROOT}/include/QImageWidget/qimagewidget.h
)

set(SOURCES
    ${MOC}
    qimagewidget.cpp
)

add_library(qimagewidget ${SOURCES})
target_link_libraries(qimagewidget Qt5::Widgets)
