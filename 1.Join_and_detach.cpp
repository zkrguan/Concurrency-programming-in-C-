#include <thread>
#include <iostream>
#include <algorithm>

/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
/// By writting it down, you will always find the way home. 
///	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//							
//1. Thoughts on the detach join
	// CPP referenece website has limited explanation on the detach only .
	// What is the difference between the join()?
	// 
	/*
		When a thread is about to die( inside the destructor ):
			Std:: terminate will be called
					if the thread was NOT JOINED
					AND was NOT DETACHED either with
			Then the program will crush

		So to learn a good habit, we should always think about:
			is my std::thread joined? or detached? before the thread dies.


		Join will make the main wait for the task to complete
			You lanched the thread like spaceX.
			You will need to write join after the main runs other codes until you need it.
			( don't have to actually write it right after launched and wait)
		But the detach won't let the main wait for the task to complete at all.

		Detach will NOT wait for the task assigned on the thread to complete
			Thinking about the shooting a missle to the target.
			Launched the thread,
			And detach() is actually like launching the missle head part.
			Then we just let it run and do the other things in the main.
			But how do you know it is done?

		I will have to learn more and know how to check if it is done...


		To be continued
	*/

int main() {
	int res1 = 0;
	int src_arr1[] = { 10,10,10,10,10,10,10,10 };
	auto add = [](int& res, int src[]) ->int& 
	{	std::for_each(src, src + 8, [&](const int src) {res += src; }); 
		return res; 
	};

	std::thread det_thread (add, std::ref(res1), src_arr1);
	det_thread.detach();

	//// g++ might work, but on visual studio's compiler, this will be optimised//
	//// So we have the line after next line//
	///*std::this_thread::sleep_for(std::chrono::nanoseconds(10000));*/
	//for (size_t i = 0; i < 10000; i++)
	//{
	//	std::cout << "";
	//}

	std::cout << "det_thread val is =======>>>>" << res1 << std::endl;
	return 0;
}