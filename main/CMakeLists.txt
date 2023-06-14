idf_component_register(SRCS "http_request_handle.c"
                            "app_main.c"
                            "WateringManager.cpp"
                            "WateringManagerWrapper.cpp"
                        INCLUDE_DIRS "."
                        EMBED_FILES assets/css/bootstrap.min.css
                                    assets/js/bootstrap.bundle.min.js
                                    assets/js/chart.min.js
                                    assets/my_index.html)              
