
include_directories(
  model
)

add_executable(NNCNN4SegReluMaxPoolAEPH NNCNNLabeler.cpp)
target_link_libraries(NNCNN4SegReluMaxPoolAEPH ${LIBS})
