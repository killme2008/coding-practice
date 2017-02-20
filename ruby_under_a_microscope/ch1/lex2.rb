require 'ripper'
require 'pp'
#ripper is not parser, it can't find error.
code = <<STR
10.times do |n
  puts n
end
STR

puts code
pp Ripper.lex(code)
