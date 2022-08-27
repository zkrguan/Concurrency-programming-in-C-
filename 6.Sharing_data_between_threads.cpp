#include <thread>
#include <algorithm>
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>
#include <string>
#include <vector>
#include <algorithm> 
#include <numeric> 
#include <chrono>
#include <latch>
#include <barrier>
/*
	After previous two notes about mutex, we learned how to protect the shared data. However, sometimes, we
	need to synchronize actions on separate threads.
	One thread might need to wait for another to complete a task before the first thread can complete its own.

	In this note, conditional varibale, futures, latches, barriers will be covered.

*/

// Waiting for an event or other condition //
//#define BLK1

// Conditional variable wait(), notify_once()// 
//#define BLK2

// Futures and one-off events //
// 1. std::async detail notes //
//#define BLK3
// 2. std::packaged_task<> // 
//#define BLK4
// 3. Manually create std::promise and std::future
#define BLK5

// sync with latches and barrier
#define BLK6

#define BLK7
#define BLK8


#ifdef BLK1
/*
	If a thread is waiting for another thread to complete a task, it has several options.

	Option 1. It could keep checking a flag in shared data (protected by a mutex) and have the second thread set
	the flag when it completes the task.
		This is really waste computer power,
			1. thread consume valuable processing time by checking the flag constantly
			2. if the waiting thread is running, it limits the resources for the thread being waited for.
				( which means you need to wait for more time)
*/

/*
	Option 2. A second option is to have the waiting thread sleep for short periods between the checks using the
	std::this_thread::sleep_for() function
*/



bool flag;
std::mutex m;
void wait_for_flag()
{
	std::unique_lock<std::mutex> lk(m);
	while (!flag)
	{
		lk.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		lk.lock();
	}
}
/*
	This is slightly better than the first option,
		1. The thread does not waste processing time while it is sleeping.
	However,
		1. Sleep too short in between checks and the thread still wastes processing time checking
		2. Sleep too long would cause a delay. ( in a game, this could cause dropped frames )



*/

/*
	Option 3 :
		PREFERRED OPTION====>
			Use the facilities from C++ standard lib to wait for an event.
			The follows code blocks would be covered this topic.
*/



#endif // BLK1



// Conditional variable// 
#ifdef BLK2
/*
	standard library provides two implementations of a condtion variable:
		std::condition_variable
		std::condition_variable_any

	Both of them need to work with a mutex in order to provide appropriate sync.

		std::condition_variable is limited to work with std::mutex	(light weighted)
		std::condition_variable_any can work with anything like a "mutex" ( more expensive but more flexible )
*/
// Added these prototypes so you won't be distracted by IDE // 
class data_chunk;
bool more_data_to_prepare();
data_chunk prepare_data();
void process(data_chunk& data);
bool is_last_chunk(data_chunk& src);


std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;
void data_preparation_thread()
{
	while (more_data_to_prepare())
	{
		data_chunk const data = prepare_data();
		{
			std::lock_guard<std::mutex> lk(mut);
			data_queue.push(data);
		}
		//---------------
		// Communication starts here.
		// It calls the notify_one() from condition_variable's member method
		//		This will basically notify the waiting thread it is ready
		data_cond.notify_one();
		//----------------

		// Also, the scope for pushing data is quite small// 
		// When it is out of the scope, the mutex unlocked then you notify all. 
	}
}
void data_processing_thread()
{
	while (true)
	{	// Lock the mutex// 
		// Why unique_lock? ???
		std::unique_lock<std::mutex> lk(mut);

		/* The condition_variable's wait method
				Notice wait in C++ 11 allows to write a lambda inside
				the lambda checks if the data_queue is empty//
				If not empty, that means still have some data for processing//

			Wait checks the condition ( by calling lambda )
				If not satisfied ( wait unlocks mutex and puts the thread in a waiting state
				If the condtion variable is notified by notify_one() from the data_preparation_thread()
					The threads wakes up, checks the lambda again.
					if satisfied: returning from the wait() with the mutex still locked.
		*/
		//----------------
		data_cond.wait(
			lk, [] {return !data_queue.empty(); });
		//----------------
		data_chunk data = data_queue.front();
		data_queue.pop();
		// Now you see why you neeed a unique lock// 
		// The waiting thread must unlock the mutex while it's waiting and lock it again// 
		// But std::lock_guard does not have this kind of flexibility. 
		// If the mutex keeps unlock during the sleeping, the data_prepation thread wouldn't be able to lock the mutex 
		// to add an item to the queue, and the lambda from the wait() would never returns false//
		lk.unlock();
		process(data);
		if (is_last_chunk(data))
			break;
	}
}
/*
	The lambda in the wait() can be replaced with the other functions or anything callable.
	BUT:
		During a call to wait(), a condition variable may check the supplied condtion any number of times.
		However, the mutex remains locked and return imediately if the condtion returns true.

	Spurious wake ( false awake )
		If the thread was waked up because the other thread changed the condtion, but this is not what you expected.
			( Data race is normally the reason. )

	It is strongly recommended that do not put a function with side effects in the wait argument.
		Because the side effects would occur multiple times.

*/

/*
	Also, a busy wait is unwanted in any cases, that's why the condtion_variable is an optimization over a busy wait
*/


#endif // BLK2



/*
	C++ standard library models this with the future libarary.

	Future is designed for one-off event with something called a future.
	If a thread needs to wait for a specific one-off event, it somehow obtains a future representing
	that event.

	The thread can periodically wait on the future for short periods of time to see if the event
	has occurred while performing some other tasks.
	( This is like playing mobile games while waiting for your GF getting ready )

	1. A future may have data associated with it or may not.
	Once an event has happened, the future can't be reset.

	2. Two types of futures ( just like ptr )
		unique futures std::future<>
		shared futures std::shared_future<>

		std::future is the one and only instance that refers to its associated event

		std::shared_future may refers the same event with multiple instances.
			All instances will be ready at the same time.
			They may all access any data associated with the event.


		std:future<void>
		std::shared_future<void>
		These two are used when there is no associated data.

	3. Futures are used to communicate between threads,
	   but the future objects themselves don't provide sync accesses.

	   If multi threads need to access a single future, mutex should be used to protect the access.

*/

#ifdef BLK3
//async//
/*
	std::async

	A function template.
		This will start an async task which you don't need the result right away.

	async returns a future object, eventually hold the return value of the function

	use future.get() on the returned future object, the thread blocks until the future is ready and
	then returns the value.

*/
//int find_the_answer_to_ltuae();
//void do_other_stuff();
//int main()
//{
//	// A basic implementation model of async//
//	std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
//	do_other_stuff();
//	std::cout << "The answer is " << the_answer.get() << std::endl;
//}



/*
	async allows you pass arguments to the funciton ( just like thread())
		Copy only unless you use std::ref()
*/
struct X
{
	void foo(int, std::string const&);
	std::string bar(std::string const&);
};
X x;

//calling the member method supplied with the argument (this) ==> &x, int == > 42, const string& hello//
auto f1 = std::async(&X::foo, &x, 42, "hello");
// Could also call the functino by passing the x as a copy //
auto f2 = std::async(&X::bar, x, "goodbye");

struct Y
{
	double operator()(double);
};
Y y;
// calling the functor Y by creating a functor object and supplies with a double//
auto f3 = std::async(Y(), 3.141);
// Calling the functor's object by passing the reference of the y, and supplies the argument as the second// 
auto f4 = std::async(std::ref(y), 2.718);
X baz(X&);
// calling the baz function by passing the functionName and the argument// 
std::async(baz, std::ref(x));

class move_only
{
public:
	move_only();
	move_only(move_only&&);
	move_only(move_only const&) = delete;
	move_only& operator=(move_only&&);
	move_only& operator=(move_only const&) = delete;
	void operator()();
};
// Calling the functor by creating a functor object by invoking the functor constructor// 
auto f5 = std::async(move_only());


/*
	async allows you to choose the
		The std::launch => launching a thread on the task right away.

			std::launch::deferred to indicate that the function call is to be deferred until
				either wait() or get() is called on the future

			or even giving two options ( second is default )
				std::launch::deferred | std::launch::async

	Be aware that if the function call is deferred, it may never run.
*/
// Launch the new thread and call the function right away //
auto f6 = std::async(std::launch::async, Y(), 1.2);
// Run the task in wait() or get()
auto f7 = std::async(std::launch::deferred, baz, std::ref(x));
// Implementation chooses //
auto f8 = std::async(3
	std::launch::deferred | std::launch::async,
	baz, std::ref(x));
// Implementation chooses //
auto f9 = std::async(baz, std::ref(x));
// Invoke deferred function //
f7.wait();

#endif // BLK3


#ifdef BLK4
/*
	This is a templated class

	std::packaged_task <> ties a future to a function or callable object.

	When the std::packaged_task<> is invoked, it calls the associated funciton or callable object
	and makes the future ready. With the returned value stored as the associated data, this can be used
	as a building block for thread pools.

	The template parameter for std::packaged_task<> is a funciton signature.
	Like std::packaged_task< void () > for a function taking no parameter with no return value
		The type does not have to match excatly because the implicit cast will happen.

	The Parameter of the constructor => must be anything callable which matches the template argument
*/
double accum(double* beg, double* end, double init) {
	return  std::accumulate(beg, end, init);
}
//________---------------------------_______________________
void task(std::promise<double>& p, double* beg, double* end, double init) {
	// in the function using the promise.set_value(task(args..))
	// BTW it only takes values unlike std::bind, or std::thread.
	p.set_value(accum(beg, end, init));
}
//________---------------------------_______________________
int main() {
	//chrono::steady_clock::time_point vstart = chrono::steady_clock::now();
	std::vector<double> vec(10000000, 0.5);
	//chrono::steady_clock::time_point vend = chrono::steady_clock::now();
	//auto vdur = chrono::duration_cast<chrono::milliseconds>(vend - vstart);
	//cout << "It took: " << vdur.count() << " milliseconds to complete" << endl;

	/*
	  To adpat your code in the multithread way
	  First is divide the work before you do anything
	*/
	double* first = &vec[0];
	double* half = first + vec.size() / 2;
	double* last = first + vec.size();
	// creating packaged task obejcts //
	std::packaged_task<double(double*, double*, double)> pt1(accum);
	std::packaged_task<double(double*, double*, double)> pt2(accum);

	// using packaged_task.get_future will return the future back from an object // 
	auto f1 = pt1.get_future();
	auto f2 = pt2.get_future();

	// pt2 and pt1 are function objects//
	// You have to move the packaged_task instead of simply supply in it)
	// When creating a thread with a packaged tasks//
	std::thread t1(std::move(pt1), first, half, 0.0);
	std::thread t2(std::move(pt2), half, last, 0.0);

	// using the future.get() method to get the result from the shared state // 
	double result = f1.get() + f2.get();

	t1.join();
	t2.join();
	std::cout << "The value of result is: " << result << std::endl;
}

#endif // BLK4



#ifdef BLK5

#endif // BLK5

#ifdef BLK6
/*
	A latch is a sync object that becomes ready when the counter decremented to zero.
		Once it is ready, it stays ready until the end of the life time.
		Light weighted for a series of event occurs

	Latch is good for resolving the thread starting problem.
	Also, this is not copyable ( just like mutex )

*/
//-----------------------
//int main() {
//	// make a latch and counter set 2
//	std::latch myLatch(2);
//	// arrive and wait for the latch // 
//	// So this thread is blocked and waiting for latch counter reaches 0 // 
//	std::thread t1([&]() {
//		myLatch.arrive_and_wait();
//		std::cout << "Hello from t1 thread\n";
//		});
//
//	std::cout << "Hello from main thread\n";
//	myLatch.arrive_and_wait();
//	std::cout << "before join\n";
//	t1.join();
//	std::cout << "after join\n";
//	std::cout << "Hello again from main thread\n";
//}
//-----------------------

// or 
//-----------------------
//int main() {
//	// make a latch and counter set 1
//	std::latch myLatch(1);
//	// this time just wait for the latch // 
//	// So this thread is blocked and waiting for latch counter reaches 0 // 
//	std::thread t1([&]() {
//		myLatch.wait();
//		std::cout << "Hello from t1 thread\n";
//	});
//
//	std::cout << "Hello from main thread\n";
//	myLatch.arrive_and_wait();
//	std::cout << "before join\n";
//	t1.join();
//	std::cout << "after join\n";
//	std::cout << "Hello again from main thread\n";
//}
//-----------------------


//*****************************************************


/*
	std::barrier is a template
	barrier is more heavy weighted compared to latch

	A barrier is reusable sync component
		Each thread can only decrement the counter once per cycle
		When the thread arrive at barrier, they are blocked until all of the thread involved have arrived
		Then they got all released. Then the threads can arrived at the barrier again.

*/

int main() {
	// the syntax is 
	// std::barrier BarierName (counterNum, anyCallable has no exception) // 
	// Second argument is actually the completion function //
	// When the barrier comes down, the function will be executed // 
	std::barrier my_barr(2, []() noexcept {puts("Green Light is on. Go ahead!"); });
	std::thread t1(
		[&]() {
			std::cout << "t1 is setting up\n";
			my_barr.arrive_and_wait();
			std::cout << "Barrier down, t1 is running\n";
		});
	std::cout << "main is setting up\n";
	my_barr.arrive_and_wait();
	std::cout << "Barrier down, t2 is running \n";
	t1.join();
}


#endif // BLK6
