#include "coroutine.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

int Coroutine::coroutine_num;
Coroutine::ptr Coroutine::main_coroutine;
Coroutine::ptr Coroutine::current_coroutine;
std::queue<Coroutine::ptr> Coroutine::delete_queue;
std::unordered_map<int, Coroutine::ptr> Coroutine::co_pool;

# pragma region constructor
Coroutine::Coroutine() {
	srand(time(NULL));

	this->m_id = 0;
	this->m_state = CoroutineState::running;
	this->cb_func = nullptr;
	this->cb_args = nullptr;
	this->is_destroyed = false;

	this->m_fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);

	if (this->m_fiber == nullptr) throw "Failed to initialize coroutine environment";
}

Coroutine::Coroutine(call_back func, void* args, size_t stack_size) : cb_func(func), cb_args(args) {
	if (main_coroutine == nullptr) {
		Coroutine::ptr p(new Coroutine(), deleter);
		co_pool.insert({ 0, p });
		main_coroutine = p;
		current_coroutine = p;
		coroutine_num = 1;
	}

	int rand_init_id = rand() % MAX_COROUTINE_NUM;
	int id = rand_init_id + 1;
	while (id != rand_init_id && co_pool.count(id)) id = (id + 1) % MAX_COROUTINE_NUM;

	if (id != rand_init_id) m_id = rand_init_id;
	else throw "failed to create coroutine, the number of coroutines is up to limit";

	stack_size = stack_size ? stack_size : default_stack_size;
	m_fiber = CreateFiberEx(stack_size,
		stack_size,
		FIBER_FLAG_FLOAT_SWITCH,
		coroutine_main_func,
		&m_id
	);

	this->is_destroyed = false;
	m_state = CoroutineState::init;
}
#pragma endregion


void Coroutine::yield() {
	if (m_id == 0) throw "The operation is invalid to the main coroutine";

	if (get_state() == CoroutineState::running) {
		m_state = CoroutineState::suspend;
		current_coroutine = main_coroutine;
		main_coroutine->m_state = CoroutineState::running;
		SwitchToFiber(main_coroutine->m_fiber);
	} else {
		throw "Coroutine state error!";
	}
}

// *this method will suspend the executing coroutine
void Coroutine::resume() {
	if (m_id == 0) throw "The operation is invalid to the main coroutine";

	if (get_state() == CoroutineState::init || get_state() == CoroutineState::suspend) {
		current_coroutine->m_state = CoroutineState::suspend;
		current_coroutine = shared_from_this();
		current_coroutine->m_state = CoroutineState::running;
		SwitchToFiber(current_coroutine->m_fiber);

		if (current_coroutine == main_coroutine) Coroutine::auto_delete();
	} else {
		throw "Coroutine state error!";
	}
}

void Coroutine::close() {
	if (m_id == 0) throw "The operation is invalid to the main coroutine";

	//std::cout << "co_pool erased: " << m_id << std::endl;
	co_pool.erase(m_id);
	delete_queue.push(shared_from_this());
	coroutine_num--;

	m_state = CoroutineState::term;

	if (current_coroutine == shared_from_this()) {
		current_coroutine = main_coroutine;
		main_coroutine->m_state = CoroutineState::running;
		SwitchToFiber(main_coroutine->m_fiber);
	}
}


Coroutine::ptr Coroutine::get_this() {
	return shared_from_this();
}

CoroutineState Coroutine::get_state() {
	return m_state;
}

int Coroutine::get_id() {
	return m_id;
}

void Coroutine::destroy_self() {
	DeleteFiber(this->m_fiber);
}

Coroutine::ptr Coroutine::create_coroutine(call_back func, void* args, size_t stack_size) {
	Coroutine::ptr cp(new Coroutine(func, args, stack_size), Coroutine::deleter);
	co_pool.insert({ cp->m_id, cp });
	coroutine_num++;
	return cp;
}

void CALLBACK Coroutine::coroutine_main_func(LPVOID lpFiberParameter) {
	int curID = *(int*)lpFiberParameter;
	Coroutine::ptr co = co_pool[curID];

	co->m_state = CoroutineState::running;
	co->cb_func(co->cb_args);

	co->close();
	current_coroutine = main_coroutine;
	SwitchToFiber(main_coroutine->m_fiber);
}

Coroutine::ptr Coroutine::get_current_coroutine() {
	return current_coroutine;
}

int Coroutine::get_coroutine_number() {
	return coroutine_num;
}

void Coroutine::auto_delete() {
	while (!delete_queue.empty()) {
		Coroutine::ptr cp = delete_queue.front();
		delete_queue.pop();
		cp->destroy_self();
	}
}

void Coroutine::deleter(Coroutine* co) {
	if (!co->is_destroyed && co->m_id != 0) {
		throw "You can't delete a running coroutine";
	}
	if (co->m_id == 0) {
		//	co->destroy_self();
	}
	delete co;
}