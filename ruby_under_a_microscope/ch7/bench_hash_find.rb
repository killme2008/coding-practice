# coding: utf-8
## 测试不同大小的 hash 查找元素的耗时
require 'benchmark'

21.times do |exponent|
  target_key = nil

  size = 2 ** exponent

  hash = {}
  (1..size).each do |n|
    index = rand
    target_key = index if n > size/2 && target_key.nil?
    hash[index] = rand
  end

  GC.disable

  Benchmark.bm do |bench|
    bench.report("retrieving an element from a hash with #{size} elements 10000 times.") do
      10000.times do
        val = hash[target_key]
      end
    end
  end

end
