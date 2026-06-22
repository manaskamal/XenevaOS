extern "C" {
    // Stub out the floating point functions that Clang builtins normally provide
    
    double __floatsidf(int i) { return (double)i; }
    double __floatunsidf(unsigned int i) { return (double)i; }
    float __floatunsisf(unsigned int i) { return (float)i; }
    int __fixdfsi(double d) { return (int)d; }
    float __divsf3(float a, float b) { return a / b; }
    double __muldf3(double a, double b) { return a * b; }
    double __adddf3(double a, double b) { return a + b; }
    double __subdf3(double a, double b) { return a - b; }
    double __divdf3(double a, double b) { return a / b; }
    
    int __gtdf2(double a, double b) { return a > b ? 1 : (a < b ? -1 : 0); }
    int __ltdf2(double a, double b) { return a < b ? -1 : (a > b ? 1 : 0); }
    int __ledf2(double a, double b) { return a <= b ? 0 : 1; }
    int __eqdf2(double a, double b) { return a == b ? 0 : 1; }
    int __nedf2(double a, double b) { return a != b ? 1 : 0; }
    
    float __truncdfsf2(double d) { return (float)d; }
    double __floatdidf(long long i) { return (double)i; }
    
    long long __fixdfdi(double a) { return (long long)a; }
    unsigned long long __fixunsdfdi(double a) { return (unsigned long long)a; }
    unsigned int __fixunsdfsi(double a) { return (unsigned int)a; }
    
    int __gesf2(float a, float b) { return a >= b ? 0 : -1; }
    int __gtsf2(float a, float b) { return a > b ? 1 : (a < b ? -1 : 0); }
    int __lesf2(float a, float b) { return a <= b ? 0 : 1; }
    int __ltsf2(float a, float b) { return a < b ? -1 : (a > b ? 1 : 0); }
    int __eqsf2(float a, float b) { return a == b ? 0 : 1; }
    int __nesf2(float a, float b) { return a != b ? 1 : 0; }
    
    float __addsf3(float a, float b) { return a + b; }
    float __subsf3(float a, float b) { return a - b; }
    float __mulsf3(float a, float b) { return a * b; }
    
    double __extendsfdf2(float a) { return (double)a; }
}
