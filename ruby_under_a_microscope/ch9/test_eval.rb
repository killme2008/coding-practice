a = 2
b = 3
str  = "puts "
str += " a +"
str += "b"

eval(str)

def get_binding
  a = 10
  b = 5
  binding
end
eval str,get_binding
