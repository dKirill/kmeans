# Intro
Basic header only kmeans clustering implementation

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
