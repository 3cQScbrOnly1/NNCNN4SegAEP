
include_directories(
  model
)

add_executable(NNCNN4SegTanhMinPool NNCNNLabeler.cpp)
target_link_libraries(NNCNN4SegTanhMinPool ${LIBS})
