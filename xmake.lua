add_rules("mode.release", "mode.debug")


add_requires("qt6widgets 6.8.0", "qt6network 6.8.0")

includes("dieselformats")

includes("tools/lookuptool")
includes("tools/xmbparser")
includes("tools/banditsdecryptor")
includes("tools/packagesdumper")

target("DieselFormatsPlayground")
  set_languages("cxxlatest")
  set_exceptions("cxx")
  set_symbols("debug")

  add_includedirs("./tools/dieselformatsplayground/src")
  add_files("./tools/dieselformatsplayground/src/**.cpp")
  add_headerfiles("./tools/dieselformatsplayground/src/**.h")

  add_defines("WIN32_LEAN_AND_MEAN")

  add_includedirs("./dieselformats/src")
  add_deps("DieselFormats")
  set_runtimes(is_mode("debug") and "MDd" or "MD")
target_end()

for _, file in ipairs(os.files("tests/test_*.cpp")) do
  local test_name = path.basename(file)
  if test_name ~= "test_shared" then
  target(test_name)
    set_kind("binary")
    set_default(false)
    add_files("tests/" .. test_name .. ".cpp")
    add_files("tests/test_shared.cpp")
    add_includedirs("./dieselformats/src")
    add_includedirs("./tests")
    add_deps("DieselFormats")
    set_runtimes(is_mode("debug") and "MDd" or "MD")
    add_tests("exec", {trim_output = true, pass_outputs = {"test passed"}, fail_outputs = {"test failed:*"}})
  target_end()
  end
end