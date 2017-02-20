code = <<END
for i in 0..5
  puts i
end
END

puts RubyVM::InstructionSequence.compile(code).disasm
