
StaticLibrary('wsa',
        [Glob('src/*.c'), Glob('component/simple_websocket_client/*.c'), Glob('utility/*/*.c')],
        LIBS = ['libwebsockets', 'ssl', 'crypto', 'pthread'], 
        CPPPATH = ['utility/md5', 'utility/log', 'utility/json', 'component/simple_websocket_client/', 'src/', 'board/centos_64/prebuilt/libwebsockets/include', 'board/centos_64/prebuilt/openssl/include'], 
        LIBPATH = ['board/centos_64/prebuilt/libwebsockets/lib', 'board/centos_64/prebuilt/openssl/lib/'])

Program('wsa_sample',
        'sample/sample.c',
        LIBS = ['wsa', 'libwebsockets', 'ssl', 'crypto', 'pthread'], 
        CPPPATH = ['utility/md5', 'utility/log', 'utility/json', 'component/simple_websocket_client/', 'src/', 'board/centos_64/prebuilt/libwebsockets/include', 'board/centos_64/prebuilt/openssl/include'], 
        LIBPATH = ['./', 'board/centos_64/prebuilt/libwebsockets/lib', 'board/centos_64/prebuilt/openssl/lib/'])

Install('./', 'sample/led.json')
