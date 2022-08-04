

/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
/// By writting it down, you will always find the way home. 
///	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//							
//2. Launching a thread

/*
	A thread can be constructed as:
		std::thread threadName {anythingCallable, args...}
		args are the anythingCallable.
		
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		1.  By default, the arguments are copied into the internal storage
			They can be accessed by newly created thread of execution. 
			Then passed to the callable as Rvalues.
			Pass a ref? You need the std::ref() wrapper to make it work like a ref..
		2.  Parameter positions do matter.
			If you have bunch of parameters, check your args and make sure their positions matched. 
			(Normally the intelsense will warn you if the arguments and parameters are mismatched)
		3.  Remember std::bind() 
		    Isn't the syntax kind of similar while invoking the constructor of std::thread().
		    (I know the bind will return a new func, but thread() will return you a thread object as well)
		4.  Supplying arguments => they are actually moved not exactly copied.
			There would be ownership transfer involved. (shall be discussed in the next code)
			Same as the thread objects, they are moveable too, but not copyable ( just like unique_ptr)
			(Just like what we learned from the database classes, one thread object points to one task)
		( 3 and 4 will be discussed more in the next note)
*/


