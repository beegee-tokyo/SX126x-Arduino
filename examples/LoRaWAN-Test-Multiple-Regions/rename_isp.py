Import("env")

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}

build_tag = "ISP_"

version_tag = defines.get("SW_VERSION")

env.Replace(PROGNAME="../../../Generated/ISP_SW_V%s" % (version_tag))
