#!/usr/bin/ruby

VSTGUI_DIR = File.expand_path(ARGV[0])
OBJ_DIR = File.expand_path(ARGV[1])
UNITTEST_EXE = File.expand_path(ARGV[2])
OUTPUT_DIR = File.expand_path(ARGV[3])
SCRIPT_DIR = File.expand_path(File.dirname(__FILE__))
puts "Generating Code Coverage with:"
puts "VSTGUI_DIR=" + VSTGUI_DIR
puts "OBJ_DIR=" + OBJ_DIR
puts "UNITTEST_EXE=" + UNITTEST_EXE
puts "SCRIPT_DIR=" + SCRIPT_DIR

def removeDataFromTrace(infoFile, directory)
  puts "Removing test coverage from #{directory}"
  system ("./lcov --remove #{infoFile} --output-file #{infoFile} #{directory}/*")
  Dir.foreach(directory) { |path|
    if (path != ".." && path != ".")
      d = File.join(directory, path)
      if (File.directory?(d))
        removeDataFromTrace(infoFile, d)
      end
    end
  }
end


begin

  system (UNITTEST_EXE)

  if ($? != 0)
    raise "unittest error"
  end

  Dir.chdir (SCRIPT_DIR)

  COVERAGE_FILE = "vstgui-coverage.info"

  system ("./lcov --capture --no-external --base-directory #{VSTGUI_DIR} --directory #{OBJ_DIR} --output-file vstgui-coverage.info")
  system ("./lcov --remove #{COVERAGE_FILE} --output-file #{COVERAGE_FILE} #{VSTGUI_DIR}/vstgui_mac.mm")
  removeDataFromTrace(COVERAGE_FILE, "#{VSTGUI_DIR}/uidescription/expat")
  removeDataFromTrace(COVERAGE_FILE, "#{VSTGUI_DIR}/tests/unittest")
  removeDataFromTrace(COVERAGE_FILE, "#{VSTGUI_DIR}/lib/platform")
  # system ("./lcov --remove vstgui-coverage.info --output-file vstgui-coverage.info #{VSTGUI_DIR}/tests/unittest/**")
  if ($? != 0)
    raise "lcov error"
  end


  puts "Generating report..."
  system ("./genhtml vstgui-coverage.info -t 'VSTGUI Test Coverage' --num-spaces 4 --demangle-cpp --output-directory #{OUTPUT_DIR}")

  if ($? != 0)
    raise "genhtml error"
  end

ensure

  Dir.foreach(OBJ_DIR) { |f| 
    ext = File.extname(f)
    if (ext == ".gcda")
      File.delete (File.join(OBJ_DIR, f))
    end
  }

end

