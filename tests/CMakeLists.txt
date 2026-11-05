set(CMAKE_AUTOMOC ON)

function(imagen_add_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE imagen_core Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

imagen_add_test(tst_geometry)
imagen_add_test(tst_config)
imagen_add_test(tst_hotkeys)
imagen_add_test(tst_outputs)
