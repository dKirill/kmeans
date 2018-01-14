# Intro
basic header only kmeans clustering implementation
tested on XCode9

# how to build&autorun tests
## makefile
```
mkdir build-make && cd build-make
cmake .. -DCMAKE_BUILD_TYPE=Release
make tests
```

## xcode
```
mkdir build-xcode && cd build-xcode
cmake .. -GXcode -DCMAKE_BUILD_TYPE=Release
xcodebuild -target tests configuration Release
```

# Regards
[CTPL](https://github.com/vit-vit/CTPL/) - The thread pool header only library used