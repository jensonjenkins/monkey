# monkey

### As of current iteration only supports:
- identifier definitions
- function defintion and calls
- arithmetic operations
- comparators (!=, ==, <, >)
- conditional statements
- datatypes: 64-bit signed ints, bools, null

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



