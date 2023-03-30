#设置需要编译的源文件列表
#set(SRC_LIST ./muduo_server.cpp)

#设置可执行文件最终存储的位置   PROJECT_SOURCE_DIR 工程的根目录
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#把.（当前目录）指定路径下的所有源文件名字放入变量名 SRC_LIST里面
aux_source_directory(. SRC_LIST) 
aux_source_directory(./db DB_LIST) 
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)

#表示生成可执行文件 server  由SRC_LIST变量所定义的源文件编译而来
add_executable(server ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

#表示生成server这个可执行文件  还需要链接muduo_net muduo_base pthread这三个库文件
target_link_libraries(server muduo_net muduo_base mysqlclient hiredis pthread)
