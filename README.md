# monkey

As of current iteration only supports:
    - basic function defintion and calls
    - arithmetic operations
    - !=, ==, <, > comparators
    - conditional statements
    - 64-bit signed ints 
    - bools
    - assignment (let a = 5;)

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



