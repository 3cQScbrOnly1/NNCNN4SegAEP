
include_directories(
  model
)

add_executable(NNCNN4SegTanhAvgPool NNCNNLabeler.cpp)
target_link_libraries(NNCNN4SegTanhAvgPool ${LIBS})
