
env=Environment()

env.ParseConfig("bash -c 'pkg-config --cflags --libs mozilla-plugin || pkg-config --cflags --libs iceape-plugin'")
env['CCFLAGS']="-Wall"
env['CXXFLAGS']="-Wall"

env.SharedLibrary('glugin',['ns-unix.cpp','core.cpp','common.cpp'],LIBS=['GLU'])

