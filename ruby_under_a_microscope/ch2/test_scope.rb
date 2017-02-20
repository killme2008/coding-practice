x=3
p = Proc.new{
10.times do |n|
  puts n+x
end
}
p.call
