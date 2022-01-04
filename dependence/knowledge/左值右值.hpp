#include <cstring>
 class Sample {
  int *ptr; // large block of memory
  int size;
 public:
  Sample(int sz=0) : ptr{sz != 0 ? new int[sz] : nullptr}, size{sz} 
  {
     if (ptr != nullptr) memset(ptr, 0, sz);
  }
  // copy constructor that takes lvalue 
  Sample(const Sample& s) : ptr{s.size != 0 ? new int[s.size] :\
      nullptr}, size{s.size}
  {
     if (ptr != nullptr) memcpy(ptr, s.ptr, s.size);
     std::cout << "copy constructor called on lvalue\n";
  }

  // move constructor that take rvalue
  Sample(Sample&& s) 
  {  // steal s's resources
     ptr = s.ptr;
     size = s.size;        
     s.ptr = nullptr; // destructive write
     s.size = 0;
     cout << "Move constructor called on rvalue." << std::endl;
  }    
  // normal copy assignment operator taking lvalue
  Sample& operator=(const Sample& s)
  {
   if(this != &s) {
      delete [] ptr; // free current pointer
      size = s.size;

      if (size != 0) {
        ptr = new int[s.size];
        memcpy(ptr, s.ptr, s.size);
      } else 
         ptr = nullptr;
     }
     cout << "Copy Assignment called on lvalue." << std::endl;
     return *this;
  }    
 // overloaded move assignment operator taking rvalue
 Sample& operator=(Sample&& lhs)
 {
   if(this != &s) {
      delete [] ptr; //don't let ptr be orphaned 
      ptr = lhs.ptr;   //but now "steal" lhs, don't clone it.
      size = lhs.size; 
      lhs.ptr = nullptr; // lhs's new "stolen" state
      lhs.size = 0;
   }
   cout << "Move Assignment called on rvalue" << std::endl;
   return *this;
 }
//...snip
}; 