class Quote
  p self
  p Module.nesting
  class << self
    p self
    p Module.nesting

    def class_method
      p self
      p Module.nesting
    end
  end
end
Quote.class_method
