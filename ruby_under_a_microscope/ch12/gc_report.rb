GC::Profiler.enable

1000000.times do
  obj = Object.new
end

GC::Profiler.report
