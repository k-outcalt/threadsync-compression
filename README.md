zip program with multithreading. 
Developers: Bryce Chinn, Katherine Outcalt, and Andrew Varela
2.14.2021

We parallelize our program by creating a thread pool and assigning threads
to files. The actual compression of the files using (run length encoding)
RLE is done in parallel, however the output is synchronized among threads 
(using semaphores) to preserve the initial order. 

The number of threads created is equal to the number of processors configured
by the operating system.

Each thread efficiently performs each piece of work by using memory-mapped
files to map the input files into the address space and then accessing
bytes of the input file via pointers. 

We coordinate multiple threads by treating the compression as an unbounded
buffer problem, where the buffer contains the tasks to be compressed.
Each task contains a pointer to the mapped file along with the size of
the file (in bytes) and an integer to store when the task was added to the
queue (relative to the other tasks). 

The threads mimicked a producer-consumer problem with an unbounded buffer.
The "producer" in this case is the main thread and the "consumers" are the 
threads that do the compression work. We used one semaphores and a mutex lock 
to solve the critical section problem and a vector of semaphores to  
enforce the ordering of each thread's output.

Kat was the main code driver for this project. Andrew focused on testing and
research. Bryce contributed to brainstorming and conceptualization.
