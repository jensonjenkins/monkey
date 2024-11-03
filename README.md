# monkey

partial implementation of Thorsten Ball's "Writing an Intepreter in Go" in C++.

### supports:
- identifier definitions
- function defintion and calls
- arithmetic operations
- comparators (!=, ==, <, >)
- conditional statements
- datatypes: 64-bit signed ints, bools, null
- static arrays + indexing

### can't:
- mutate arrays or any variable for that matter
- a lot more

### Code sample:
``` ml
let some_function = fn(x, y, z, func) {
    return x + func(y, z);
}

let my_func = fn(x, y) {
    return x * y;
}

some_function(5, 2, 3, my_func);

```
``` ml
let iterate = fn(start, end, array, sum) {
    if(start < end) {
        return iterate(start + 1, end, array, sum + array[start]);
    } else {
        return sum;
    }
}

let a = [1, 2, 3, 4, 5];

return iterate(0, 5, a, 0);

```

### To build tests and executables: 
```sh
~/monkey$ mkdir build
~/monkey$ cmake --build ./build/ 
```

### To run REPL: 
```sh
~/monkey$ ./build/repl

```

### To run executable: 
```sh
~/monkey$ ./build/monkey < [[Your .ky file]]

```

### Run examples: 
```sh
~/monkey$ ./build/monkey < ./examples/conditionals.ky
~/monkey$ ./build/monkey < ./examples/basic_functions.ky

```
### TODOs:
- Custom allocator (stack based allocators for fixed size objects)
- Organize lifetime of object::object
- Support for stdout
- More data structures (maps, sets)
- More builtin_fns (filter, sort, print, etc)



