idf_component_register(SRCS "GlobalConfig.cpp" "GlobalConfigTable.cpp" "TimeManger.c" "http_request_handle.c"
                            "app_main.c"
                            "WateringManager.cpp"
                            "WateringManagerWrapper.cpp"
                        INCLUDE_DIRS "."
                        EMBED_FILES assets/css/bootstrap.min.css
                                    assets/js/bootstrap.bundle.min.js
                                    assets/js/chart.min.js
                                    assets/my_index.html)              
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)