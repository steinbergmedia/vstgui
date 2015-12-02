#!/usr/bin/ruby

VSTGUI_DIR = File.expand_path(ARGV[0])
OBJ_DIR = File.expand_path(ARGV[1])
UNITTEST_EXE = File.expand_path(ARGV[2])
SCRIPT_DIR = File.expand_path(File.dirname(__FILE__))
puts "Generating Code Coveragew with:"
puts "VSTGUI_DIR=" + VSTGUI_DIR
puts "OBJ_DIR=" + OBJ_DIR
puts "UNITTEST_EXE=" + UNITTEST_EXE
puts "SCRIPT_DIR=" + SCRIPT_DIR

begin

  system (UNITTEST_EXE)

  if ($? != 0)
    raise "unittest error"
  end

  Dir.chdir (SCRIPT_DIR)

  system ("./lcov --capture --no-external --directory #{VSTGUI_DIR} --output-file vstgui-coverage.info")
  system ("./lcov -r vstgui-coverage.info --output-file vstgui-coverage.info vstgui/uidescription/expat/*")
  system ("./lcov -r vstgui-coverage.info --output-file vstgui-coverage.info vstgui/tests/unittest/*")
  if ($? != 0)
    raise "lcov error"
  end


  system ("./genhtml vstgui-coverage.info -t 'VSTGUI Test Coverage' --num-spaces 4 --demangle-cpp --output-directory out")

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

