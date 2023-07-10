add_executable(${PROJECT} 
   main.cpp
   depth_shader.cpp
   )

target_include_directories(${PROJECT}
   PRIVATE
   ${CMAKE_SOURCE_DIR}/source/include/)   

   