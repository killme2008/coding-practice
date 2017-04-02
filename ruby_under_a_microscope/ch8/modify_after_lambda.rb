def message_function
  str = "The quick brown fox"
  func = lambda do |animal|
    puts "#{str} jumps over the lazy #{animal}."
  end
  str = "the sly brown fox"
  func
end

f = message_function
f.call('dog')
