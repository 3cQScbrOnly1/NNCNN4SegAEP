
include_directories(
  basic
)


add_subdirectory(NNCNN4Seg)
add_subdirectory(NNCNN4SegTanhMaxPool)
add_subdirectory(NNCNN4SegSigmoidMaxPool)
add_subdirectory(NNCNN4SegReluMaxPool)

add_subdirectory(NNCNN4SegTanhAvgPool)
add_subdirectory(NNCNN4SegTanhMinPool)
add_subdirectory(NNCNN4SegTanhTriPool)