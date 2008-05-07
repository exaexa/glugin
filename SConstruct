
env=Environment()

env.ParseConfig('pkg-config --cflags --libs mozilla-plugin')
env['CCFLAGS']="-Wall"
env['CXXFLAGS']="-Wall"

env.SharedLibrary('glugin',['ns-unix.cpp','core.cpp','common.cpp'],LIBS=['GLU'])

