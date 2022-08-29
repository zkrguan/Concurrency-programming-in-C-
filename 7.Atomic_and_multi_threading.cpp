/*
	so far we used the mutex and locks to code in order to avoid the data race. 

	Now std::atomic is widely used in the lock free programming. 

	And like the name, it is like the nuke in C++, if you don't code it right, the nuke will be triggered

	"Joggling Razor Blades" 
						-- Herb Sutter
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
	for(unsigned x{0};x<100;x++)
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
#ifdef BLK2
/*
	
*/
int main() {
	int x = 3;
	int y = 2;
	y += x;
	const char* msg = "Hello\n";
	int flag = 1;
	int b = flag+1;

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


#endif // BLK2