g++ 01.NumberSequence.cpp 02.Wrapper.cpp -g -o Run.out --std=c++1z `pkg-config --libs gtest` -lgtest_main `pkg-config --cflags gtest` && ./Run.out
