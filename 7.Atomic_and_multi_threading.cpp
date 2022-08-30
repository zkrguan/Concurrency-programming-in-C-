/*
	so far we used the mutex and locks to code in order to avoid the data race.

	Now std::atomic is widely used in the lock free programming.

	And like the name, it is like the nuke in C++, if you don't code it right, the nuke will be triggered

	"Joggling Razor Blades"
						-- Herb Sutter


	Some vids

	https://www.youtube.com/watch?v=DS2m7T6NKZQ -- Micheal Wong, CPP conference 2015

*/

#include <thread>
#include <iostream>
// without lock, simply resolve the data race // 
//#define BLK1

#define BLK2
#define BLK3
#define BLK4
#define BLK5
#define BLK6
#define BLK7
#define BLK8
#define BLK9 
#define BLK10


/*
   If you don't know or understand data race,
   please go to the mutex notes for resolving data race

*/


/*
	1. The code you write is not excatly equals to the code executes after compilation
	Because there are numerous optimizations happened after the compilation starts.


	myCode =======> Compiler=================> CPU========================> Cache=============> Finally the result will go the main memory
					Reordering					Out of order execution		Prefetch
					Loop unrolling				Branch Prediction			Buffering


	2. Memory model we have seen
	Multiple threads working on one memory

	3. Solutions about solving data race
		A. sharing less
		B. Mutex and locks
		C  std::atomic ( manip memory orders )
		D. Abstraction ( Hopefully I will have some notes on this )

*/

#ifdef BLK1
std::atomic<int> x_int(0);

void incrementer() {
	x_int++;
}

void task() {
	for (unsigned x{ 0 }; x < 100; x++)
		incrementer();
}

int main() {
	std::thread t1(task);
	std::thread t2(task);
	std::thread t3(task);
	t1.join();
	t2.join();
	t3.join();
	std::cout << "The final res of x_int is " << x_int << std::endl;
}


/*
	What excatly happened here?
	??????????????????????????
	incrementer has 3 steps read and increment and write
	std::atomic made those 3 steps happens in the atomic way.
		So no threads will be accessing the variable while one thread is doing the three steps.
		So there is no data race.
*/


#endif // BLK1

/*
	So that's it end of my note

	Of course no man, otherwise, there would not be this many of senpai coders warn us again and again.
	It is actually used more than that.


	First of all, what can be atomic?
		Almost everything pretty much.
		Boolean, int, float, pointers, smart pointers.
		( But be aware, no reference)

		User defined type?
		Yes, but with limitations:
			1. it has to be trivially copyable ( have both copy constructor and copy assginment operator
				and no user-defined copy and move constructor and no user defined assigment operator
			2. continous block of memory
			3. No virtual functions or virtual base classes
		( so std::vector is not atomic because of the first rule)

	Already sounds like a hassel to implement in your code? This is just a start

*/

/*
*	syntax:
*
*
	std::atomic  has
						member functions ----------- specialised and general
						constants
						memory orders

	how to define?

	1. std::atomic<typeName>variableName;
	2. You can not use the = to initialize
	3. Can only use {} () to initialize


	std::atomic is not copyAssignable.
		So you can either define another temp variable to trans the value from one to another
		Or use .load and .store methods
		e.g.
			std::atomic<int> a{1},b{2};
			a.store(b.load());

		Actually the these two ways are essentially equals to each other.

	.exchange(newValue) method
		old_value = atomicVariable.exchange(new_value);

		Read and write at the same time.
		e.g.
			int y = 2, z = 3;
			std::atomic<int> atomic_x{1};
			z= atomic_x.exchange(y);
			// Now x =2, y = 2, z =1;
			// z is still equals to the old value of the x

		// This is actually three line of atomic operations in one line (read, write, and the result will come out//


	.compare_exchange_strong() or .weak()
		bool success = atomicVariable.compare exchange(expected, desired)

		e.g.:
		if( atomic_x == expected){
			atomic_x = desired;
			return true;
		}
		else{
			expected = atomic_x;
			return false;
		}

		desire will be set as the new value if the expected matches the current value
		otherwise, the expected will be set to the current value.

		Why we need such a compilcated bs?
		e.g.:
			int faa (const int& a){
				return a + 10;
			}
			x = faa(x);

			How do you make this atomic?
				t = x.load();
				t = f(t);
				x.store(t);
			But in a multithread environment, how do you make this three lines atomic?
			Using mutex?
			You don't have to. You can use this compare_exchange_strong();
				auto old_val = g_atomicX.load();
				while(!g_atomicX.compare_exchange_strong(old_val,f(oldX)));

				This will get you the most updated values in the multi-thread coding environemnt.
*/

/*
	All atomic types ( apart from the std::atomic_flag ) may be implemented using mutexes or other locking operations.

	Atomic types are also allowed to be sometimes lock-free.
		( not always lock free)
	e.g.:
		if a given strcuture is misaligned objects of the same types, they must use locks.

		std::atomic<type>::is_lock_free() will check the objects is lock free.
			( or is_always_lock_free() )

*/

/*
* Specialised functions:
*
	fetch_add, fetch_sub (int, float, ptr)
	fetch_and, fetch_or, fetch_xor, ( int only )
	++----++ (int ptr)
	+= -= (int float ptr)
	&= |= ^= (int only)

*/


#ifdef BLK2



#endif // BLK2




/*
	We were saying the atomic does not only make the operation atomic, it has more than that.
	What is it?

	Introduce with the program order first=>

	Sequence before :

	Single_thread_environment:
		With single thread, the program order is sequence-before, executed one by one.
		But the sequence may be reordered by compiler and cache.
		However, the result must be the same as reordered. And the code before the output has privacy.
		The same as the program order.

	Multi_thread environment:
		One thread can see through other threads. Meaning the privacy will be taken out, the reordered program code will be illustrated.
		This is called side-effects or reordering
		Execution order is undefined.
		It only has some sequential consistency.

*/
#ifdef BLK3
/*

*/
int main() {
	int x = 3;
	int y = 2;
	y += x;
	const char* msg = "Hello\n";
	int flag = 1;
	int b = flag + 1;

	std::cout << "msg: " << msg;
	std::cout << "B: " << b << " , y: " << y << std::endl;
	/*
		Single thread environment
		msg: Hello
		b: 2,y: 3
	*/

	/*
		let's say if the following thread's code is relying on the code above.
	*/
	//const char* text = "Hi\n";
	//while (flag!=1){
	//	text = msg;
	//}

	// If there is no reordered happens, the msg was set before the flag = 1. 
	// The text in the other thread should be "Hello\n"
	// But if the reordered happens, and the flag = 1 happened before msg = "Hello\n";
	// The text is empty 

	// How to resolve this then? We need std::atomic 


}


#endif // BLK3




#ifndef BLK4
/*
	Memory model and atomic:

	"Only when you get close to the machine that the precise details of the memory model matter"
					by Anothony Williams my idol

	Basically atomic allows lower level sync operations that will reduce to one or two CPU instructions.


	Memory models:

		Two aspect of the memory model:
		1.Basic structrual aspect => variables and the other stuff lays in memory.
			All data in C++ is made up of objects, ( but you can not derived from int )
			Whatever the type is, an object is stored in one or more memory locations.
			As we all know, struct is one object that consists several sub objects.
				Every object occupies at least one memory location
				Variables of fundamental types such as int or char occupy exactly one memory
					location, doesn't matter their size, or array.
		2.Concurrency aspects =>
		// TD//

	Modification orders:
		Every object in C++ has a modification order => all the writes to that object from all threads
		in the program, starting with the object initialization.

		In most cases, this order will vary between runs, but given the execution of the program all
		threads in the system must agree on the order.

		Speculative execution are not permitted ( like the privacy mentioned from the above BLK ) because once
		a thread has seen a particular entry in the modification order, subsequent reads from that thread must return
		later values, subsequent writes from that thread object must occur later in the modification order.



	Memory Model, Atomic and concurrency:
		Reordering:
				When a program executes instructions, especially memory reads and writes,
				in an order that is different than the order specified in the program's source code.

		Memory model:
				Describes how memory reads and write may appear to be executed relative to their program order.
				" represents a contract between the implementers and the programmers to ensure that most
				  programmers do not have to think about the details of modern computer hardware "
																							- Stroustrup
				CPU actually changes your code.
				( "how dare you!" --- Micheal Wong, CPP con 2015)

		Thread switching:
				Execution of a sequential program
				software
				A processor can run a thread

		C++ memory model:
			It tells how threads interact with memory.
			What assumptions the compiler is allowed to make when generate the code.



			Isolation:



			Visibility:



			Ordering:


		Message shared memory:
			Writes are explicitly communicated between pairs of threads.
			via a lock or an atomic variable.
			The mechanism is called acquire and release.



		In order to avoid race condition, the order must be enforced between accessing the data from
			two threads.

		You have seen the mutex, but the use of the atomic operations can enforce an ordering too.
			atomic is not setting the order of which touches the memory location first. It is just
			making the accessing the memory location operation atomic.

		An atomic operation is an indivisible operation, which means you can not observe such an operation
		half-done from any thread in the system. It is either done or not done.
		If the lcad operation that reads the value of an object is atomic, and all modifications to that object are also atomic.

		On the other hand, non-atomic operation might be seen as half-done by another thread.
		If the non-atomic operation is composed of atomic operations. (e.g. assignment to a struct with atomic memebrs )
		then other threads may observe some subset of the constituent atomic operations as complete, but others as no yet started.
		( potential data race could occur )

		std::atomic <> class has an optional memory-ordering argument which is one of the values of the std::memory_order ( enum )
		This argument is used to specify the required memory-ordering semantics.
		std::memory_order_relaxed,
		std:: memory_order_acquire,
		std::memory_order_consume,
		std::memory_order_acq_rel,
		std::memory_order_release,
		std::memory_order_seq_cst.( strongest ordering also default )

		store operations can have memory_order_relaxed, memory_order_release, or memory_order_seq_cst
		load operations can have  memory_order_relaxed, memory_order_consume, memory_order_acquire, or memory_order_seq_cst
		read modify write can have memory_order_relaxed, memory_order_consume, memory_order_acquire, memory_order_release, memory_order_acq_rel, or memory_order_seq_cst



*/


#endif // !BLK4
