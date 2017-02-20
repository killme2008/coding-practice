module Professor
  def name
    "Prof. #{super}"
  end
end
class Mathematician
  attr_accessor :name
  prepend Professor
end

p  = Mathematician.new
p.name = 'Johann Carl Friedrich Gauss'

p p.name
