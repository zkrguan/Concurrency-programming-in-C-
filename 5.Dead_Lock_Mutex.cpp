#include <thread>
#include <string>
#include <iostream>
#include <mutex>
#include <exception>
#include <stdexcept>
#include <deque>
/*
	What is a dead lock?

	Think about two dudes.
	One has bullets, the other one has rifle.
	Now two grizzly bears jumped out from the bush.
	Both of them want to shoot the bear front of them.
	Neither of them wants to let the other guy shoot first.
	What would happen?
	They won't even shoot out one bullet out until they die.

	Deadlock is just like this:
	This problem arises when there are two or more mutexes exist.
	And your operation needs to lock two or more of them.

	How to avoid it?
	The following codes will be the advice from those OG coders.
*/

// Always lock mutex in the same order // 
// Extremly simple example of dead lock//

// 1. First thing you need to care lock the mutex in a fix order ( or you call it sequence)
//#define BLK1

// 2A std::lock() and transferring mutex ownership
//#define BLK2

// 2B std::scope_lock() do two things at one shot
//#define BLK3

// 3 Last a few advice on dead locks
//#define BLK4


// Unique_lock first visit
//#define BLK5

// Transferring mutex ownership between scopes with unique_lock
//#define BLK6

// Locking at an appropriate granularity
#define BLK7

#define BLK8
#define BLK9

// Made some modifications //
#ifdef BLK1
/* Deadlock */
// Always lock mutex in the same order // 
// Extremly simple example of dead lock//
// First thing you need to care lock the mutex in a fix order ( or you call it sequence)
int main() {
	std::mutex m1;
	std::mutex m2;
	auto f1 = [&m1, &m2]() {
		std::lock_guard<std::mutex> lg1(m1);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::lock_guard<std::mutex> lg2(m2);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	};
	auto f2 = [&m1, &m2]() {
		std::lock_guard<std::mutex> lg1(m2);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::lock_guard<std::mutex> lg2(m1);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	};
	std::thread thread1(f1);
	std::thread thread2(f2);
	thread1.join();
	thread2.join();
}
// Solution: lock the mutexes in a fixed order
#endif // BLK1

// Move on to a harder version from Anthony Williams
#ifdef BLK2
class some_big_object;
void swap(some_big_object& lhs, some_big_object& rhs);
class X
{
private:
	some_big_object some_detail;
	std::mutex m;
public:
	X(some_big_object const& sd) :some_detail(sd) {}

	friend void swap(X& lhs, X& rhs)
	{	// Checking self swapping//
		if (&lhs == &rhs)
			return;
		// std::lock() will lock the two mutexes, and two lock guards are defined after 
		std::lock(lhs.m, rhs.m);
		// std::adopt_lock will only transfer the mutex ownership rather than locking it. 
		// std::adopt_lock parameter is supplied in addition to the mutex to indicate to 
		// the std::lock objects that the mutexs are already locked
		// the ownership of the existing lock on the mutex rather than attempt to lock the mutex 
		// in the constructor

		// Why transferring the ownership? So the lock_guard will unlock the mutex after function
		// ends or there is an exception triggerred. 
		std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
		std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);
		swap(lhs.some_detail, rhs.some_detail);
	}
};

#endif // BLK2


/*
	C++ 17 provides a more elegant way to handle this.
	By using the scope_lock<>
*/

#ifdef BLK3
void swap(X& lhs, X& rhs) {
	if (&lhs == &rhs)
		return;
	//	scope_lock's constructor argument is variadic template 
	//		Which means you can pass many mutexs as template parameter
	// This is like using the lock and transferring the ownership at once // 
	// So it is exception safe because it follows RAII rules//
	// Also, c++ 17 makes you save some template argument <std::mutex>
	std::scoped_lock guard(lhs.m, rhs.m);
	swap(lhs.some_detail, rhs.some_detail);
}


#endif // BLK3


/*
	Deadlock does not only occur with locks, you can create deadlock with two threads and no locks
	by having each thread call join()

	This is like two threads are waiting for each other to finish, but neither of them can finish because
	they are waiting for the other to finish.


	-- don’t wait for another thread if there’s a chance it’s waiting for you.
																	Anthony Williams
*/

#ifdef BLK4

/*
	Last a few advices:
		Avoid nested locks

		Avoid calling user-supplied code while holding a lock

		Acquire locks in a fixed order

		Use a lock hierarchy
*/


// Try to create a dead_lock senario that two threads are waiting for each other

#endif // BLK4



/*
	std:: unique_lock ( compare with the std::lock_guard )
		std::defer_lock as the second arg to indicate that mutex should remain unlocked on construction.
		Later on, you can also call std::lock() by passing the unique_lock objects.

	Beware that unique locks take more space and slower than the other approach we used in BLK3
	Unique_lock objects don't have to own their associated mutexes, the ownership of a mutex can be
	transferred between instances by moving it around. 
	Don't use it unless you actually need the flexibility. 

*/
#ifdef BLK5
class some_big_object;
void swap(some_big_object& lhs, some_big_object& rhs);
class X
{
private:
	some_big_object some_detail;
	std::mutex m;
public:
	X(some_big_object const& sd) :some_detail(sd) {}
	friend void swap(X& lhs, X& rhs)
	{
		if (&lhs == &rhs)
			return;
		std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
		std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
		// Right now the mutexes are not locked yet// 
		// So use the std::lock passing the unique_lock instead of mutexes//
		// Also, unique_lock has a few options for you to manually lock or unlock //
		// These will manip the flag inside a unique_lock instance..//
		// So inside the destructor, it will check the flag to ensure unlock() is called//
		std::lock(lock_a, lock_b);
		swap(lhs.some_detail, rhs.some_detail);
	}
};


#endif // BLK5


/* moiving mutex ownership between scopes */
#ifdef BLK6
/*
	Moving a unique_lock sometimes is automatic:
		When returning an instance from a funcitn, or when you called std::move()
		Ownership transfer is automatic if the source is an rvalue
		When it is lvalue, you have to use std::move()

	Just like the objects such as thread, unique_ptr, unique_lock is only moveable but 
	not copyable. 




*/
std::unique_lock<std::mutex> get_lock()
{	// a global mutex. 
	// 
	extern std::mutex some_mutex;
	// lk is declared inside the funciton, so it can be returned directly without move//
	std::unique_lock<std::mutex> lk(some_mutex);
	prepare_data();
	return lk;                                      
}
void process_data()
{
	// by calling get_lock, the ownership was directly transferred to the new unique_lock
	// The mutex is remained locked, so there would not be any thread to mainp the data at the same time//
	std::unique_lock<std::mutex> lk(get_lock());    
	do_something();
}

#endif // BLK6




#ifdef BLK7
/*
	The granularity of a lock	=>	describes the amount of data protected by a single lock.
	A fine_grained lock protects a small amount of data
	A coarse-grained lock protects a large amount of data

	Also, lock the right mutex for the right operation is important.
	Only lock the mutex when you are touching the shared_data.
	Prepare your data before you lock the mutex.
	Manip the shared data right after lock the mutex.

	( This is why people use unique_lock, because the coders can lock when they need and unlock after
	they manip the shared_data )

*/
// Inside the operator ==
// we are locking the mutex once a time by calling the get_detail funciton
// What could be the problem?
// Once your lock_guard out of function scope, it will be unlocked. 
// Then the data could be maniped by the other threads. 
// how can you assure the returned boolean is accurate? 
class Y{
private:
	int some_detail;
	mutable std::mutex m;
	int get_detail() const
	{
		std::lock_guard<std::mutex> lock_a(m);   
			return some_detail;
	}
public:
	Y(int sd) :some_detail(sd) {}
	friend bool operator==(Y const& lhs, Y const& rhs)
	{
		if (&lhs == &rhs)
			return true;
		int const lhs_value = lhs.get_detail();    
			int const rhs_value = rhs.get_detail();    
			return lhs_value == rhs_value;             
	}
};


#endif // BLK7


