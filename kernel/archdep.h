#define hlt __asm__("hlt\n\t")  

extern void fast_memcpy(void* d, void* s, int count);