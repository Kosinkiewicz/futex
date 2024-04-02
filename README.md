# futex library 

Author: Leszek Kosinkiewicz<kosinkiewicz@gmail.com>

There are mutex and barrier implementation in c++14 based on kernel linux futex object.
The futex::Mutex is about 30-60% faster then the gcc std::mutex.

This is header only libray + UT under google test.
This library is under the Apache License Version 2.0.
Pls. read LICENCE file.

For help type: 'make help'.
To test type: 'make test'.
To install type: 'make install'.

what you need: gcc (c++14), gtest(google test)

