#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

// 全局的watcher观察者  zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) // 回调的消息类型和会话相关的消息类型
    {
        if (state == ZOO_CONNECTED_STATE) // zkclient和zkserver连接成功
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem); // 给信号量资源加1
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源  MySQL_Conn
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /*
    zookeeper_init：
    第一个参数host格式要求是e.g. "127.0.0.1:3000，
    第二个参数watcher_fn，需要定义回调函数
    第三个参数会话的超时时间
    zookeeper_mt：多线程版本
    zookeeper的API客户端程序提供了三个线程：
    1.API调用线程   独立线程
    2.网络I/O线程   pthread_create  poll   独立线程
    3.watcher回调线程    独立线程
    */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0); // 信号量初始化0
    // 创建句柄成功，并不是连接zkserver成功
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem); // 首先阻塞态，主线程看到有资源，就往下执行
    std::cout << "zookeeper_init success!" << std::endl;
}

// 在zkserver上根据指定的path创建znode节点，默认是0（永久性节点）
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    // 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag)    // 表示path的znode节点不存在
    {
        // 创建指定path的znode节点
        flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE,
                          state, path_buffer, bufferlen);
        if (flag == ZOK)
        {
            std::cout << "znode create success... path:" << path << std::endl;
        }
        else
        {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error... path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}
// 根据参数指定的znode节点路径path，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if(flag != ZOK)
    {
        std::cout << "get znode error... path:" << path << std::endl;
        return "";
    }
    else
    {
        return buffer;
    }
}
