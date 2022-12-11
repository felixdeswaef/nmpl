cmake_minimum_required(VERSION 3.23)
project(nmpl)

set(CMAKE_CXX_STANDARD 14)

file(COPY keypoints1.key keypoints2.key keypoints3.key keypoints4.key image1.pgm image2.pgm image3.pgm image4.pgm alpr_out.txt  DESTINATION .)
add_executable(nmpl main.c main.h util2.c util2.h)
