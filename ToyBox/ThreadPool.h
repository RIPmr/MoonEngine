#pragma once
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <iostream>
#include <thread>
#include <vector>
#pragma comment(lib, "pthreadVC2.lib")

namespace MOON {
	#define MAX_THREADSNUM 6

	class MThread {
	private:
		std::thread m_t;

	public:
		template<class T>
		explicit MThread(void (T::*deleMethod)(void*), T* tasker, void* arg) {
			m_t = std::thread(deleMethod, tasker, arg);
		}
		~MThread() { if (m_t.joinable()) m_t.join(); }

		MThread(MThread const&) = delete;
	};

	class ThreadPool {
	public:
		static std::vector<std::thread*> pool;

		inline static void WaitAllThreadExit() {
			for (std::thread* th : pool) {
				if (th->joinable()) th->join();
			}
		}

		inline static void Clean() {
			for (std::thread* th : pool) delete th;
			pool.clear();
		}

		// method need to be static
		inline static void CreateThread(void* (*method)(void*), void* arg) {
			pthread_t newThread;
			int ret = pthread_create(&newThread, NULL, method, arg);
			if (ret) std::cout << "thread error! pthread_create error: error_code=" << ret << std::endl;
			else std::cout << "new thread created!" << std::endl;
		}

		template<class T, class P>
		inline static void CreateThread(void (T::*deleMethod)(P&), T* tasker, P& arg) {
			std::thread* newThread = new std::thread(deleMethod, tasker, arg);
			//newThread->detach();
			pool.push_back(newThread);
		}
	};

}