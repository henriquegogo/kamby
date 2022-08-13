# Kamby Language
Kamby Programming Language is a Lisp dialect with some conventions to create a lange more intuitive and compact.

## Lisp? But it doesn't look like
Internaly the implementation follows some basic concepts like S-expressions and car/cdr as any Lisp language.
Kamby has some conventions to make the syntax more friendly:
- Anything starting with a new line and finishing in end of line is considered an expression
- An item formed by 2 or less punctuation characters, will create an expression formed by (punct previous next). Ex.: 2 + 2 => (+ 2 2) ... something == anything => (== something anything)
- Blocks will be evaluated if is the first item of expression.

### Actions
- def key 'value' (Append 'key' in stack)
- key := 'value'  (Append 'key' in stack - syntax sugar for 'def')
- key = 'value'   (Edit last 'key' in stack)
- del key         (Remove last 'key' from stack)
- if (condition) { 'first' } (else_condition) { 'second' }
- while {condition} { 'Do this' }
- for {initialization} {condition} {increment} { 'Do this' }
- len [9 8 7 6]   (Return number of items)
- puts key 'or text'

### Operators
```
+ - * /
&& || == != >= <= > <
+= -=
```

#### Special operators
Operator "+" will sum two numbers, concatenate two strings, append a node to list or merge two lists.
```ruby
puts 1 + 2               # 3
puts "Kamby" + "Lang"    # KambyLang
[1 2] += 3               # [1 2 3]
list = ([1 2] += [3 4])  # [1 2 3 4]
```

Operators "==" and "!=" can be used to compare numbers and strings.
```ruby
if 4 != 2 { puts 'NOT OK' }
if 'two' == "two" { puts 'OK' }
```

### Types
- number = 123
- string = "Text"
- expression = ( "Eval immediately" )
- block = { "Eval when called" }
- list = ['Any' 'item' 9 5 3 'any' ['type']]

#### Blocks
Blocks are similar to expressions but will be evaluated only when called as first item of an expression. Others items will be added as "arg" inside block scope.
```ruby
def say { puts arg }
say "Hello"                     # Run { puts "Hello" }
say { message := "Scoped var" }
```

#### Lists
```ruby
list = [9 8 7 6]  # Creates a list
list += 5         # Append 5 to list
list :: {. 1}     # Get first item
```

#### Objects
```ruby
# Objects are list of attributions
obj = [
    name := 'My name'
    age := 20
]
obj :: {        # The '::' operator will apply 'obj' as the context to block
    name = 'Your name'
}
puts obj :: {name}
```

## How to run
```sh
make
./kamby
```

## Example
```ruby
message = 'Hello, World!'
puts 'Message:' message
puts "Sum:" 1 + 2 + 3

if false {
    puts 'Initial condition'
} else {
    puts 'Last condition'
}

count = 0
while {count < 3} {
    count += 1
    puts 'Number:' count
}

list = ['first' 'second' 'third']
list += 'fourth'
puts list . 4    # Return 'fourth'
```

## Known issues / TODO
- VM / Bytecode
- Blocks are running when defined

## License
MIT
