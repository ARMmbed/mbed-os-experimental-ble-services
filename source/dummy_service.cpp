// Dummy definitions for setting up CMake

namespace dummy { 
void foo() { }
}


int Factorial(int n) { 
    if (n < 0) { 
        return -42;      
    }

    if (n == 0) { 
        return 1;
    } else { 
        return n * Factorial(n - 1); 
    }
}