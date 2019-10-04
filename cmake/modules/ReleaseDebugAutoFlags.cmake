## debug / release autoManagement


if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE  "-O2")
    set(CMAKE_C_FLAGS_DEBUG  "-Wall")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -Wall")

    set(CMAKE_CXX_FLAGS_RELEASE  "-O2")
    set(CMAKE_CXX_FLAGS_DEBUG  "-Wall")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -Wall")
else()
    set(CMAKE_C_FLAGS_RELEASE  "-O2 -Wno-unused-parameter")
    set(CMAKE_C_FLAGS_DEBUG  "-g -Wall -Wextra -pedantic -fstack-protector-all -Wno-unused-parameter")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO " -Wall -g -O2")

    set(CMAKE_CXX_FLAGS_RELEASE  "-Wall -Wextra -Wno-unused-parameter -O2")
    set(CMAKE_CXX_FLAGS_DEBUG  "-g -Wall -Wextra -Wno-unused-parameter -fstack-protector-all")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO " -Wall -Wextra -Wno-unused-parameter -g -O2")
endif()