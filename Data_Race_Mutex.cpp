#include <thread>
#include <string>
#include <iostream>
#include <mutex>
#include<exception>
#include<stdexcept>
/*
	Like the famous metaphor alaways says. Imagine you are living in a shared
	rental place where you and your roommates are sharing the bathroom.

	You do not want to share the washroom with a second person, while you are
	using it.

	Same like thread and concurrency programming.

	You do not want to have two threads manipulating(not reading) the same variable at the
	same time.

*/




// A data race senario // 
//#define BLK1

// A potential fix of the race senario// 
//#define BLK2

// How the lock_guard behaving follows RAII to handle exception//
#define BLK3
#define BLK4
#define BLK5
#define BLK6
#define BLK7
#define BLK8
#define BLK9


// An typical data race senario given by my Prof. // 
#ifdef BLK1
using namespace std;
int x = 0;

void funcA() {

	for (int i = 0; i < 10000; ++i) {
		x++;
	}
}

int main() {
	/*
		Race condition:
			Like you and the other guys lining up at Canada Computer for the GPU.
			There is only 1 RTX3080 left, but you and the other customer paid for that
			at different salespersons.
			Whether you can get that RTX3080 is totally depending how fast your sales process
			your payment and take the GPU from the inventory.

			In the real life, all possible outcomes are acceptable because they are
			all part of our lives.

			BUT, in concurrency, race conditions are not fun at all. E.G. data races
			can cause the dreaded undefined behavior.

			Also, race conditions are so hard to debug( even harder then memory leak )
			They are hard to dup because of the limited time window.

	*/

	/*
		A very typical data race
		This will make the result unpredicatable.

		4 threads will tear up the data because they are reading and changing
		the value at the same time.
	*/
	thread t1(funcA);
	thread t2(funcA);
	thread t3(funcA);
	thread t4(funcA);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	// And the results are unpredicable // 
	cout << "What is the value of x now: " << x << endl;
}
#endif // BLK1

/*
		How to avoid it?
		Simple option=> wrap the data structure with a protection mechanism to
		ensure that only one the thread can perform the modification at a time.

		Hardcore option=> lock-free programming!!!( will be introduced more in the future
		notes) ( This is kind of beyond my knowledge at this stage of study )

		Other hardcore option=> update the data structure like a data base (e.g.SQL)
		There is no direct support for STM in C++ ( probably I will start a new repo on
		this software transactional memory (STM))

*/

/*
*
	mutex:  mutual exclusion
	Before accessing a shared data structure, lock the mutex associated with
	the data. When you done with the modifying, simply unlock the mutex.
	During the lock, the thread lib ensures that the other threads will not
	touch the same mutex until it is unlocked by the current thread.
	( Imagine you are using the public washroom and lock the door )
	( Some annoying dudes would check if the washroom is still locked or not constantly)
	( no matter how he knocks, you won't open because you want to do your thing)
	( until you finished, unlock the door, he would be able to going in and lock it)



	mutexName.lock();	mutexName.unlock();
	( manual lock or unlock is not recommended because humans are forgetful.)
	( it is better to use the lock_guard class template ( just like smart ptr following the
	RAII rules ) )

*/

#ifdef BLK2

using namespace std;
int x = 0;

mutex mu;

void funcA() {
	//// 1. Traditional manual locks ( not recommended as I stated before, if you forget
	//// The program would stop running and waiting for the mu to be unlocked.//
	//mu.lock();
	//for (int i = 0; i < 10000; ++i) {
	//    x++;
	//}
	//mu.unlock();

	//// 2. Lockguard before C++17
	////	Slightly complicated to use 
	////  lock_guard<mutex> guardName (mutexName)
	//std::lock_guard<std::mutex> guard1 (mu);
	//for (int i = 0; i < 10000; ++i) {
	//	x++;
	//}
	//// Good thing about this is you don't have to worry about unlock after use//
	//// Most importantly EXCEPTION SAFE!!!!!!!!!!!!!!
	//// Exception and handling will be introduced in the BLK3

	// 3. Lockguard after C++17
	// Simplified the template argument while declaring a new thread// 
	std::lock_guard guard1 (mu);
	for (int i = 0; i < 10000; ++i) {
		x++;
	}

}

int main() {
	std::vector<std::thread> thread_vec{};
	for (size_t i = 0; i < 4; i++){
		thread_vec.push_back(std::thread(funcA));
	}
	for (thread& x : thread_vec) {
		x.join();
	}
	// And the results are unpredicable // 
	cout << "What is the value of x now: " << x << endl;
}
#endif // BLK2





#ifdef BLK3
/* 
	Strange enough no one on youtube and reference books mentioned about throw exception in 
	the thread won't be able to get caught by the main. 

	One of the explanation on stack overflow mentioned that the try and catch block is based on the 
	stack, which is called the dynamic constructs of the stack. 
	When we created a new thread, you create a brand new stack. 

	The try catch wouldn't be carried over to this new stack at all. 

	To fix this issue, introduce the exception_ptr 
*/

using namespace std;
int x = 0;
mutex mu;

// Why static? I haven't found any good explanation on this yet.//
static std::exception_ptr teptr = nullptr;

void funcA() {
	std::lock_guard guard1(mu);
	// Instead of having a try catch around the thread launches, which is not smart//
	// It is right to have the try and catch inside the task //
	try
	{
		for (int i = 0; i < 10000; ++i) {
			x++;
			if (x==100){
				// Here throw// 
				throw runtime_error ("Surprise!!!!!!\n");
			}
		}
	}
	// Catch and let the exception_ptr pointing at the exception //
	catch (...){
		teptr = std::current_exception();
	}

}

int main() {
	std::vector<std::thread> thread_vec{};
	for (size_t i = 0; i < 4; i++) {
		thread_vec.push_back(std::thread(funcA));
	}
	for (thread& x : thread_vec) {
		x.join();
	}
	// Checking if the exception_ptr has any exception inside//
	if (teptr){
		// use another try catch block to rethrow and catch and print out the message//
		try	{
			rethrow_exception(teptr);
		}
		catch (const std::exception& ex)
		{
			cerr << "Thread exited with exception: " << ex.what() << "\n";
		}
	}

	// When you run the code, you will probably ask why the result is 30100// 
	// One of the thread launched and trigger the exception from the task
	// exit from the loop and go to the catch block// 
	// Then the rest of 3 threads carried on//
	cout << "What is the value of x now: " << x << endl;
}
#endif // BLK3