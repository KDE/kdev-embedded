project(%{APPNAMEID})

set(%{APPNAMEID}_SRCS
    %{APPNAMELC}.cpp
    main.cpp
)

add_executable(%{APPNAMELC} ${%{APPNAMEID}_SRCS})
install(TARGETS %{APPNAMELC} DESTINATION bin)
