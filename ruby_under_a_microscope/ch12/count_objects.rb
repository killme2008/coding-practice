def display_count
  data = ObjectSpace.count_objects
  puts "Total: #{data[:TOTAL]}, Free: #{data[:FREE]}, Object: #{data[:T_OBJECT]}"
end

10.times do
  obj = Object.new
  display_count
end
