class Quote
  def self.me
    self
  end

  def display
    "hello"
  end
end
class << Quote
  def me2
    self
  end
end

puts Quote.me == Quote.itself
puts Quote.me == Quote.me2
puts Quote.singleton_class == Quote.me
puts Quote.singleton_class

q = Quote.new

class << q
  def display
    super << " world"
  end
  def me
    self
  end
end

puts q.display
puts q.me == q
puts q.singleton_class
puts q.me
