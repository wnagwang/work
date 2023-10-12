#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <functional>
#include <atomic>
#include <future>
#include <chrono>

class ThreadPool{

public:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator = (const ThreadPool&) = delete;

    ThreadPool(const ThreadPool&&) = delete;
    ThreadPool& operator = (const ThreadPool&&) = delete;
    
    ThreadPool(int ThreadNum):m_threads(ThreadNum),m_work(true){
        Init_Thread();
    }
    ThreadPool():m_threads(std::thread::hardware_concurrency()),m_work(true){
        Init_Thread();
    }
    template<class T, class ... Args>
    auto add(T&& func ,Args ... args){
        using returntype = typename std::__invoke_result<T,Args ...>::type;
        std::function<returntype()> task = std::bind(std::forward<T>(func),std::forward<Args>(args)...);

        auto task_work = std::make_shared<std::packaged_task<returntype()>>(task);

        TaskType m_work = [task_work](){
            (*task_work)();
        };
        m_TaskQue.push(m_work);
        m_cv.notify_one();
        return task_work->get_future();
    }
    ~ThreadPool(){
        m_work = false;
        m_cv.notify_all();

        for(auto &thread : m_threads){
            if(thread.joinable())
                thread.join();
        }
    }


private:
    using TaskType = std::function<void()>;
    std::queue<TaskType> m_TaskQue;

    std::mutex m_mutex;
    std::vector<std::thread> m_threads;

    std::condition_variable m_cv;
    std::atomic<bool> m_work;

    void Init_Thread(){
        for(int i = 0 ; i < m_threads.size() ; ++ i){
            auto work = [this,i](){
                while(m_work){
                    std::unique_lock<std::mutex> lock(m_mutex);
                    while(m_TaskQue.empty()){
                        m_cv.wait(lock);
                    }
                    TaskType func = m_TaskQue.front();
                    m_TaskQue.pop();

                    printf("id :%d  start\n",i);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    func();
                    printf("id :%d  end\n",i);
                }
            };
            m_threads[i]= std::thread(work);
        }
    }
};

template<class T>
T add (T a, T b){
    //std::cout << a << " + " << b << " = " << a+b;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return a+b;
}

int main(){
    ThreadPool* m_threadpool = new ThreadPool(4);
    int tasknum = 30;
    std::vector<std::future<int>> m_res(30);
    std::cout << "start add\n";
    for(int i = 0 ; i < tasknum ; ++ i){
        m_res[i] = m_threadpool->add(::add<int> ,i , i + 1);
    }
    std::cout << "end add\n";

    std::cout << "wait\n";
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "finish\n";

    for(int i = 0 ; i < tasknum ; ++ i){
        printf("res[%d] : %d\n" ,i,m_res[i].get()); 
    }
}