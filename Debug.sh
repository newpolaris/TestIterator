g++ 01.NumberSequence.cpp 02.Wrapper.cpp 03.Non-1-1.cpp 04.PostInc.cpp -g -o Run.out --std=c++1z `pkg-config --libs gtest` -lgtest_main `pkg-config --cflags gtest` && ./Run.out
