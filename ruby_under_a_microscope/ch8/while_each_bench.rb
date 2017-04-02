require 'benchmark'

ITERATION = 1000000

Benchmark.bm do |bench|
  bench.report("iterating from 1 to 10, one millions times") do
    ITERATION.times do
      sum = 0
      i = 1;
      while i <= 10
        sum += i
        i+=1
      end
    end
  end
end

Benchmark.bm do |bench|
  bench.report("iterating from 1 to 10 with range, one millions times") do
    ITERATION.times do
      sum = 0
      (1..10).each do |i|
        sum += i
      end
    end
  end
end
