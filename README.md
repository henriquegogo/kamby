Kamby Language
==============
A small, embeddable and convenient language for who want to use and understand
what is happening behind the scenes.

How to build
------------
You can build the binary using Makefile.

    $ make                                 # Build binary
    $ ./kamby --help                       # Display help message
    $ ./kamby --version                    # Display version information
    $ ./kamby -c script.ka > script.c      # Transpile to C
    $ ./kamby script.ka                    # Run script
    $ ./kamby                              # Run REPL

Variables stack
---------------
Variables can contain any type of data, including number, strings, lists or even
blocks of code.

    message := 'Hello, World!'  # Define a new variable
    message = 'Bye, bye'        # Edit variable value
    message := 'Hello again! '  # Append new 'message' in stack
    message = ()                # Remove last 'message' in stack
    print(message)              # Message has old value 'Bye, bye'
    
    /* or */
    
    def message 'Hello, World!'
    set message 'Bye, bye'
    def message 'Hello again! '
    del message

Variables are not unique. You can declare multiple values for the same variable
name using ":=" and append into the stack. If you want to edit the last
declaration, just use a simple "=" operator. To remove the last variable with
specific name in stack, use "del varname" or set it to "()".

You can return the value of some stack item using index instead of var name

    num1 = 12  // Change num1 value or define if inexistent
    num2 = 23
    print $0   // Return last item value from stack. 23
    print $1   // Return the second item from stack. 12

Atoms like numbers and strings can be named and not saved in the stack.
This is useful to name a value in a list item.

    list = [firstName: 'John', lastName: 'Doe', age: 25]
    print(list.firstName)       // John
    print(list.age)             // 25
    print(list.$1)              // Doe

Expressions, lists and blocks
-----------------------------
Basically those three types of lists has the same structure but act different
when executed.

    // Expressions will be evaluated an return the value
    expression = (1 + 2)
    
    // Binary operators wraps itself in an expression
    expression = 1 + 2  # Same as (1 + 2)
    
    /*
    List items are separated by spaces, comma or semicolon.
    Expressions return will be attribuited to item
    */
    list = ['first item' (1 + 2)]  # Same as ['first item', 5]
    
    // Blocks are lists of expressions that can be called after declaration
    def block {
      print(message)
    }
    block message:"Hello, World!"

    // You can access blocks arguments using $0, $1, $2, etc.
    def block {
      print($0) 
    }
    block 'Hello, World!'

    /* or */

    def block {
      0 = message:$0
      print message
    }

    /* or */

    def block {
      [message:$0].{
          print(message)
      }
    }

For source code or blocks, each line is considered an expression. To execute a
block, just call it as first item of an expression with any arguments.
The arguments are be appended to the internal block variables stack.
If there's no argument, use an empty expression "()" as argument.

    def sum { $0 + $1 }
    print(sum 2 3)
    
    /* or */
    
    sum := { first + second }
    print(sum(first: 2, second: 3))

Context
-------
Lists can be send as a context appended to global variables context.

    name := 'Global name'  // Declare a new variable in current stack
    
    person = [
      name: 'Local name'
      age: 20
    ]
    
    print(name)            // Return 'Global name'
    
    print(person.name)     // "person" is set as context
                           // "name" returns list's "name" item
    
    person.{               // Context blocks uses list as context
      name = 'New name'    // Block of code that change "person" values
      age = 30
    }
    print(person.name)     // Return 'New name'

Conditions
----------
Statements are represented by pairs of condition and execution blocks.
The last block is the else block.

    // Simple condition. Block executed
    1 == 1 ? { print('TRUE') }
    /* or */
    if 1 == 1 { print('TRUE') }
    
    // False condition. Else block is executed
    1 != 1 ? { print('FALSE') } { print('ELSE') }
    /* or */
    if 1 != 1 { print('FALSE') } else { print('ELSE') }
    
    // First is false. Second condition is true. Second block is executed
    1 != 1 ? { print('FALSE') } 2 == 2 { print('TRUE') }
    /* or */
    if 1 != 1 { print('FALSE') } 2 == 2 { print('TRUE') }
    
    // Two false conditions. Else block is executed
    1 != 1 ? { print('FALSE') } 2 != 2 { print('FALSE') } else { print('ELSE') }
    /* or */
    if 1 != 1 { print('FALSE') } 2 != 2 { print('FALSE') } { print('ELSE') }

The "else" keyword means "true" and it can be omitted.
Different from other languages, the ":" symbol can't be used as "else".

Loops
-----
While condition block is true, run execution block.

    i := 0
    while {(i += 1) <= 10} { print i }
    // Notice that the "while" condition is a block {}, not an expression ().
    // Should be a block because it will run multiple times for each iteration.
    // Expressions are evaluated once.

For each item in list, run execution block.

    list := [1, 2, 3] * { ($) * 2 }  // ($) == $0
    list * { print($) }              // 2 4 6
    /* or */
    for list { print($) }

For with range is used to iterate a range of numbers or a list.

    for 0..2 { print($) }  // 0 1 2

String and list functions
-------------------------

    split "Hello World!" " "      // ["Hello", "World!"]
    join ['Hello', 'World!'] ' '  // "Hello World!"
    length "John Doe"             // 8
    upper "John Doe"              // "JOHN DOE"
    lower "John Doe"              // "john doe"

I/O
---
Input and output functions.

    print('Enter your name:')    // Print to console
    name = (input)               // Read user input
    write 'file.txt' name        // Write name to file
    content = (read 'file.txt')  // Read file content

Load scripts and libraries
--------------------------
You can load other scripts or dynamic libraries using the "load" function.

    load 'script.ka'   // Read and evaluate scripts
    load 'library.so'  // Load a dynamic library

Dynamic libraries should have a function named "void ka_extend(Kamby \**ctx)"
that will be called to extend the context with new functions.

Overloading
-----------
Some operators are overloaded to perform different actions based on argument types.

    1 + 2                // 3
    "John " + "Doe"      // "John Doe"
    [1, 2] + [3, 4]      // [1 2 3 4]
    [1, 2] + 3           // [1 2 3]
    1 + [2, 3]           // [1 2 3]

    4 / 2                // 2
    "John Doe" / " "     // ["John" "Doe"]
    "Doe" / ""           // ["D" "o" "e"]

    3 * 2                // 6
    [1 2] * { ($) * 2 }  // [2 4]
    ["a" "b" "c"] * "-"  // "a-b-c"

Operators and keywords
----------------------

    true false
    get def set del return
    $ : := = . && || ! == != > < >= <=
    ? .. + - * / % += -= *= /= %=
    if else while for
    split join length upper lower
    print input read write load

License
-------
MIT
