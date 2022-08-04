# Kamby
Kamby Programming Language is a Lisp dialect with some conventions to create a lange more intuitive and compact.

## Lisp? But it doesn't look like
Internaly the implementation follows some basic concepts like S-expressions and car/cdr as any Lisp language.
Kamby has some conventions to make the syntax more friendly:
- Anything starting with a new line and finishing in end of line is considered an expression
- If second item is formed by 2 or less punctuation characters, this identifier will be moved to the first item of list (in Lisp, this will be considered as 'car'). Ex.: (2 + 2) => (+ 2 2) ... (something == anything) => (== something anything)
- Blocks will be evaluated if is the first item of expression.

### Actions
- def key 'value' (Append 'key' in stack)
- key := 'value'  (Append 'key' in stack - syntax sugar for 'def')
- key = 'value'   (Edit last 'key' in stack)
- del key         (Remove last 'key' from stack)
- if (condition) { 'first' } (else_condition) { 'second' }
- while {condition} { 'Do this' }
- for {initialization} {condition} {increment} { 'Do this' }
- puts key 'or text'

### Operators
```
+ - * /
&& || == != >= <= > <
```

#### Special operators
Operator "+" will sum two numbers, concatenate two strings, append a node to list or merge two lists.
```ruby
puts (1 + 2)             # 3
puts ("Kamby" + "Lang")  # KambyLang
list = [1 2] + 3         # [1 2 3]
list = [1 2] + [3 4]     # [1 2 3 4]
```

Operators "==" and "!=" can be used to compare numbers and strings.
```ruby
if (4 != 2) { puts 'NOT OK' }
if ('two' == "two") { puts 'OK' }
```

### Types
- number = 123
- string = "Text"
- expression = ( "Eval immediately" )
- block = { "Eval when called" }
- list = ['Any' 'item' 9 5 3 'any' ['type']]

## How to run
```sh
make
./kamby
```

## Example
```ruby
message = 'Hello, World!'
puts 'Message:' message
puts "Sum:" (1 + (2 + 3))

if false {
    puts 'Initial condition'
} else {
    puts 'Last condition'
}

count = 0
while {count < 3} {
    count = (count + 1)
    puts 'Number:' count
}

list = ['first' 'second' 'third']
list = (list + 'fourth')
puts (list 3)
```

## Known issues / TODO
- VM / Bytecode
- More lists methods
- Objects

## License
MIT
