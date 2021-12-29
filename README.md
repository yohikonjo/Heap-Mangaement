# Heap-Mangaement
This program allows the user to implement a library that interacts with the operating system to perform heap management on behalf of a user process. 
# Features
•	Custom malloc, realloc, calloc and free implementations
•	Heap management schemes implementations I.e. First-Fit, Next-Fit, Best-Fit, and Worst-Fit.
•	Coalescing of free memory block and splitting of unused memory blocks  
# Program Execution
The code compiles into four shared libraries and four test programs.  To build the code, change to your top-level directory and type:     
**make**   
Once you have the library, you can use it to override the existing malloc by using   
LD_PRELOAD:     
**$ env LD_PRELOAD=lib/libmalloc-ff.so tests/test1**   
The heap management schemes, libraries are:   
**Best-Fit:  libmalloc-bf.so**   
**First-Fit: libmalloc-ff.so**    
**Next-Fit:  libmalloc-nf.so**   
**Worst-Fit: libmalloc-wf.so**  
The tests folder contains tests for each heap management scheme as well as tests for the other functionalities
