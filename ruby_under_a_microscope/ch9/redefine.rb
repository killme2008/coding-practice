class Quote
  def display
    puts "The quick brown fox jumped over the lazy dog."
  end
end

module AllCaps
  refine Quote do
    def display
      puts "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG."
    end
  end
end
def test
  Quote.new.display
  using AllCaps
  Quote.new.display
end

Quote.new.display
using AllCaps
Quote.new.display

test
