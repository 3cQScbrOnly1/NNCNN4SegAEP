
include_directories(
  model
)

add_executable(NNCNN4SegEX NNCNNLabeler.cpp)
target_link_libraries(NNCNN4SegEX ${LIBS})
