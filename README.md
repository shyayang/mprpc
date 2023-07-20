# mprpc
基于muduo库、protobuf以及zookeeper作为服务配置中心的RPC分布式网络通信框架
# 概述   
使用muduo开源网络库和Protobuf数据序列化和反序列化协议，实现了高效的RPC通信。   
使用互斥量和条件变量实现线程安全的缓冲队列，实现了异步的日志输出。   
使用服务器中间件ZooKeeper的服务注册和服务发现机制，实现了分布式下的服务发现功能。   
# 预编译环境
安装protobuf  
安装zookeeper  
安装muduo库  
安装CMake集成编译环境  
