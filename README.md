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
- reassign variables
- mutate arrays or any variable for that matter
- a lot more

### Code sample:
```ml
let some_function = fn(x, y, z, func) {
    return x + func(y, z);
}

let my_func = fn(x, y) {
    return x * y;
}

some_function(5, 2, 3, my_func);

```
``` ml
let a = 5;

let counter = fn(x) {
    if(x > 100) {
        return x;
    } else {
        let foobar = 9999;
        counter(x + 1);
    }
}

counter(a);

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



