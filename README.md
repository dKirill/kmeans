# Intro
Basic header only kmeans clustering implementation

Algorithm is taken from [here](http://ilpubs.stanford.edu:8090/778/1/2006-13.pdf)

Tested on XCode9

# Preparation
Do not forget to 
```
git submodule update --init --recursive
```
after clone, since Catch2 unit test framework is included as submodule

# How to build&autorun tests
## makefile
```
mkdir build-make && cd build-make
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## XCode
```
mkdir build-xcode && cd build-xcode
cmake .. -GXcode -DCMAKE_BUILD_TYPE=Release
xcodebuild -configuration Release
```

# Regards
[CTPL](https://github.com/vit-vit/CTPL/) - The thread pool header only library used
