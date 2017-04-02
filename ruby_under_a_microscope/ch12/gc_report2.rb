GC::Profiler.enable

objs = []
1000000.times do
  objs << Object.new
end

GC::Profiler.report
