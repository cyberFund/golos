file(GLOB HEADERS
     "include/steemit/encrypt/crypto_types.hpp"
     "include/steemit/encrypt/custodyutils.hpp"
     "include/steemit/encrypt/encryptionutils.hpp"
     "${PBC_INCLUDE_DIR}/pbc/*.h")

add_executable(test_encrypt test_encryption_utils.cpp ${HEADERS})
#add_executable( test_pbc_benchmark test_pbc_benchmark.cpp ${HEADERS} )
add_library(golos_encrypt
            encryptionutils.cpp
            custodyutils.cpp
            ${HEADERS})

target_link_libraries(golos_encrypt
                      PUBLIC fc pbc cryptopp)

if(WIN32)
    target_link_libraries(test_encrypt pbc golos_encrypt ${GMP_LIBRARIES})
else()
    target_link_libraries(test_encrypt pbc golos_encrypt gmp)
endif()
#target_link_libraries( test_pbc_benchmark pbc golos_encrypt gmp )
target_include_directories(golos_encrypt
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include")
target_include_directories(test_encrypt
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../chain/include")
#target_include_directories( test_pbc_benchmark
#       PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../chain/include" )


#install( TARGETS
#   golos_encrypt
#
#   RUNTIME DESTINATION bin
#   LIBRARY DESTINATION lib
#   ARCHIVE DESTINATION lib
#)