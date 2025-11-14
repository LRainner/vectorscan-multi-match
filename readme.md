## build vectorscan
```
git clone https://github.com/VectorCamp/vectorscan.git
cd vectorscan && mkdir build && cd build
curl -O https://archives.boost.io/release/1.86.0/source/boost_1_86_0.zip && unzip boost_1_86_0.zip
cmake .. -DBOOST_ROOT=$PWD/boost_1_86_0 -DBUILD_SHARED_LIBS=OFF -DFAT_RUNTIME=OFF -DBUILD_STATIC_LIBS=ON -DCMAKE_INSTALL_PREFIX=../../hyperscan/
make && make install
```

## demo usage

### match ok demo

```
g++ -std=c++17 -O2 main_ok.cc -Ihyperscan/include -Lhyperscan/lib -lhs -o ./match_ok_demo && ./match_ok_demo
```
scratch size: 3072 bytes
database size: 16104 bytes
Total length: 21, matches: 1
     0 [0,21) pattern="<%[\s\S]+%>" text="<%out.print();int a%>"

### match err demo
```
g++ -std=c++17 -O2 main_err.cc -Ihyperscan/include -Lhyperscan/lib -lhs -o ./match_err_demo && ./match_err_demo
```
scratch size: 3982 bytes
database size: 56808 bytes
Total length: 21, matches: 0