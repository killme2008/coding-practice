module Professor
  def letcures ; end
end

class Mathematician
  attr_accessor :first_name
  attr_accessor :last_name

  include Professor
end

p = Mathematician.new
p.first_name = 'hello'
p.last_name = 'world'

p p.methods.sort

#open Professor, adds new method
module Professor
  def classroom; end
end

p p.methods.sort
