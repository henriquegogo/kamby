# Kamby
Kamby Programming Language is a Lisp dialect with some conventions to create a lange more intuitive and compact

## Lisp? But it doesn't look like
Internaly the implementation follows some basic concepts like S-expressions and car/cdr as any Lisp language.
Kamby has some conventions to make the syntax more friendly:
- Anything starting with a new line and finishing in end of line is considered an expression
- If second item is formed by 2 or less punctuation characters, this identifier will be moved to the first item of list (in Lisp, this will be considered as 'car'). Ex.: (2 + 2) => (+ 2 2) ... (something == anything) => (== something anything)

## Built-in commands
### Actions
- Any implemented yet

### Operators
```
+ - * /
&& || == != >= <= > <
```

## How to run
```sh
make
./kamby
```

## Example
```ruby
message = "Hello, world"
puts message

puts (2 + (2 * 2))

list = [one two three]
```

## Known issues / TODO
- 

## License
MIT
