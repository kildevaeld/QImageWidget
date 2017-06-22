

set(SOURCES
    main.cpp
)

add_executable(example ${SOURCES})
target_link_libraries(example Qt5::Widgets qimagewidget)

