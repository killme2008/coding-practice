# coding: utf-8
i = 0
##共用一份复制的栈帧
incr = lambda do
  puts "Increment from #{i} to #{i+1}."
  i += 1
end

decr = lambda do
  puts "Decrement from #{i} to #{i-1}."
  i -= 1
end

incr.call
decr.call
incr.call
incr.call
decr.call
