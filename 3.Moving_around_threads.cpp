#include <thread>
#include <iostream>

/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
/// |By writting it down, you will always find the way home.| 
///	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//							
//3. Moving around the ownership of a thread

/*

	P1
	================================================================================
	!!!!!!!!!!!!!!!!
	According to the C++ reference
	
		move (1) : thread& operator= (thread&& rhs) noexcept;
		rhs no longer represents any thread of execution 
		If the current object is not joinable, then the rhs will be moved to it. 
			Else, terminate will be triggered.


*/

/*
	remember the previous note:
		Thread objects are moveable, let's go a little deeper in this note.
*/

/*
	Uncomment the following code, and try to run it.
	Which line cause trigger the terminate?
*/

//void fn1() {
//	std::cout << "Hello from Func 1\n";
//};
//
//void fn2() {
//	std::cout << "Hello from Func 2\n";
//};
//
//
//int main() {
//	// launched t1//
//	std::thread t1(fn1);
//	// Moved t1's ownership to t2//
//	std::thread t2 = std::move(t1);
//	// Now t1 is empty, allocate the new thread object//
//	// t1 is NOT empty now
//	t1 = std::thread(fn2);
//	// Now launch t3, by moving the t2's ownership over// 
//	std::thread t3 = std::move(t2);
//	// Trying to move T3's ownership onto a non-empty thread object?
//	// This must be it right?
//	// t1 = std::move(t3);
//	t1.join();
//	t3.join();
//	//why is next line triggering the terminate?
//	//after move, there is nothing associated with t2// 
//	//t2 it is not joinable//
//	//So you can not join or detach it//
//	//t2.join();
//	//t2.detach();
//	return 0;
//}

/*

		P2
	================================================================================
	Now let's do some sick moves. 
	std::thread is a type of object right?
	We could have some functions that return type is std::thread right?
	A thread factory function? 
*/

//std::thread function1() {
//	return std::thread([]() {std::cout << "hello from function1\n"; });
//}
//
//std::thread function2() {
//	return std::thread([]() {std::cout << "hello from function2\n"; });
//}
//
//int main() {
//	std::thread t1 = function1();
//	std::thread t2 = std::move(t1);
//	t1 = function2();
//	t2.join();
//	t1.join();
//	return 0;
//}

/*
*	
	================================================================================
	And std::thread can be a typeName of your parameter of the function too right?
	

*/

std::thread f1(std::thread paraT) {
	auto some_work = []() {std::cout << "M inside F1 and making the thread from the parameter do the work for me\n"; };
	std::thread temp(some_work);
	if (paraT.joinable()){
		paraT.join();
	}
	//paraT.detach();
	// After you move that thread, when it is out of scope, destructor won't trigger the terminate// 
	paraT = std::move(temp);
	return paraT;
}

int main() {
	std::thread t1{};
	std::thread t2([]() {std::cout << "task 1 allocated on t2\n"; });
	// The reason why moved src object won't trigger terminate// 
	//// Just think about it like a thread object declared as by the default constructor// 
	// // This thing won't trigger any terminate when it is about to die// 
	//std::thread t3{};
	// After std::move(your_old_thread), you don't need to worry about that thread's status anymore. 
	// You don't need to join 
	// You don't need to detach
	// src object is already an empty shell ~sad// 
	t1 = f1(std::move(t2));
	t2 = f1(std::move(t1));
	t2.join();
	return 0;
}



/*

		P3
	================================================================================
	This snippet of code was written the other coders, and I found this from a book at Seneca.
	Don't worry I am not making money out of this. 

	Making a scoped_Thread wrapper for thread is actually so smart
	The destructor could actually make the thread scope safe.
*/

class scoped_thread {
	std::thread t;
public:
	 // Using the initialization area to finish the transferring thread ownership// 
	 // beatiful and elegant, this dude won't even waste one line // 
	explicit scoped_thread(std::thread src) : t(std::move(src)) {
		if (!t.joinable()){
			throw std::logic_error("no thread");
		}
	}
	// When it's out of the scope, the thread object with this wrapper will trigger the destructor//
	~scoped_thread() { t.join(); };
	// And of course, banned all the copy operations. 
	scoped_thread(scoped_thread const&) = delete;
	scoped_thread& operator = (scoped_thread const&) = delete;
};


/*

	I really hope I will something more to add, and gradually make this become to my ref book in the future 

*/