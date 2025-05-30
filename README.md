Kamby Language
==============
A small, embeddable and convenient language for who want to use and understand
what is happening behind the scenes.

How to build
------------
You can build the binary using the Makefile.

    $ make                                  # Build binary
    $ ./kamby                               # Run REPL
    $ ./kamby script.ka                     # Run script
    $ ./kamby --to-c script.ka > script.c   # Transpile to C
    $ ./kamby --help                        # Show help

Variables stack
---------------
Variables can contain any type of data, including number, strings, lists or even
blocks of code.

    message := 'Hello, World!'  # Define a new variable
    message = 'Bye, bye'        # Edit variable value
    message := 'Hello again! '  # Append new 'message' in stack
    message = ()                # Remove last 'message' in stack
    message                     # Message has old value 'Bye, bye'
    
    /* or */
    
    def message 'Hello, World!'
    set message 'Bye, bye'
    def message 'Hello again! '
    del message

Variables are not unique. You can declare multiple values for the same variable
name using ":=" and append in the stack. If you want to edit the last
declaration, just user simple "=" operator. To remove the last variable with
specific name in stack, use "del varname"

You can return the value of some stack item using index instead of var name

    num1 = 12                   // Change num1 value or define if inexistent
    num2 = 23
    $0                          // Return last item value from stack. 23
    $1                          // Return the second item from stack. 12

Atoms like numbers and strings can be named and not saved in the stack.
This is useful to name a value in a list item.

    list = [fistName: 'John', lastName: 'Doe', age: 25]
    print(list.firstName)       // John
    print(list.age)             // 25
    print(list.$1)              // Doe

Expressions, lists and blocks
-----------------------------
Basically those three types of lists has the same structure but act different
when executed.

    // Expressions will be evaluated an return the value
    expression = (1 + 2)
    
    // Binary operators wraps itself in a expression
    expression = 1 + 2
    
    /*
    List items are separated by spaces, comma or semicolon.
    Expressions return will be attribuited to item
    */
    list = ['first item' (1 + 2)]
    
    // Blocks are lists of expressions that can be called after declaration
    def block {
      print(message)
    }

For source code or blocks, each line is considered an expression. To execute a
block, just call it as first item of an expression. The arguments are be
appended to the internal block variables stack.

    def sum { $0 + $1 }
    sum 2 3
    
    /* or */
    
    sum := { first + second }
    sum(first: 2, second: 3)

Context
-------
Lists can be sent as a context appended to global variables context.

    name := 'Global name'       // Declare a new variable in current stack
    
    person = [
      name: 'Local name'
      age: 20
    ]
    
    name                        // Return 'Global name'
    
    person.name                 // "person" is set as context
                                // "name" returns list's "name" item
    
    person.{                    // Context blocks uses list as context
      name = 'New name'         // Block of code that change "person" values
      age = 30
    }
    person.name                 // Return 'New name'

Conditions
----------
Statements are represented by pairs of condition block and execution block.
The last block is the else block.

    // Simple condition. Block executed
    1 == 1 ? { print('TRUE') }
    /* or */
    if 1 == 1 { print('TRUE') }
    
    // False condition. Else block is executed
    1 != 1 ? { print('FALSE') } { print('ELSE') }
    /* or */
    if 1 != 1 { print('FALSE') } { print('ELSE') }
    
    // First is false. Second condition is true. Second block is executed
    1 != 1 ? { print('FALSE') } 2 == 2 { print('TRUE') }
    /* or */
    if 1 != 1 { print('FALSE') } 2 == 2 { print('TRUE') }
    
    // Two false conditions. Else block is executed
    1 != 1 ? { print('FALSE') } 2 != 2 { print('FALSE') } { print('ELSE') }
    /* or */
    if 1 != 1 { print('FALSE') } 2 != 2 { print('FALSE') } { print('ELSE') }

Loops
-----
While condition block is true, run execution block.

    i := 0
    while ((i += 1) <= 10) { print i }

For each item in list, run execution block.

    list := [1, 2, 3] ... { $0 * 2 }
    list ... { print($0) }             // 2 4 6
    /* or */
    each list { print($0) }

For loop is used to iterate a range of numbers or a list.

    for (i := 0; i < 3; i += 1) { print(i) }     // 0 1 2

I/O
---
Input and output are done using print, input, read, write functions.

    print('Enter your name:')          // Print to console
    name = (input)                     // Read user input
    write 'file.txt' name              // Write name to file
    content = (read 'file.txt')        // Read file content

Load scripts and libraries
--------------------------
You can load other scripts or dynamic libraries using the "load" function.

    load 'script.ka'                   // Read and evaluate scripts
    load 'library.so'                  // Load a dynamic library

Dynamic libraries should have a function named "void ka_extend(Kamby \**ctx)"
that will be called to extend the context with new functions.

Operators
---------

    get def set $ : := = .
    && || ! == != > < >= <=
    + - * / % += -= *= /= %=
    if each while for ? ...
    print input read write load

Operator "+" will sum two numbers, or concatenate two strings.

    1 + 2                    // 3
    "Kamby" + "Lang"         // KambyLang

License
-------
MIT
