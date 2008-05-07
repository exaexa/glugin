
env=Environment()

env['CCFLAGS']="-shared -Wall"
env['CXXFLAGS']="-shared -Wall"
env['CPPPATH']=['/usr/lib/mozilla-firefox/include/java/','/usr/lib/mozilla-firefox/include/plugin/','/usr/include/nspr/']

env.SharedLibrary('glugin',['ns-unix.cpp','core.cpp','common.cpp'],LIBS=['GLU'])

