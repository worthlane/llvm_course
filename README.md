## Установка LLVM

```
export PATH="/opt/homebrew/opt/llvm@14/bin:$PATH"
export LDFLAGS="-L/opt/homebrew/opt/llvm@14/lib"
export CPPFLAGS="-I/opt/homebrew/opt/llvm@14/include"
```

## Сборка пасса

```
clang++ Pass.cpp -c -fPIC -I$(llvm-config --includedir) -isysroot $(xcrun --sdk macosx --show-sdk-path)  -o Pass.o

clang++ Pass.o -fPIC -shared -isysroot $(xcrun --sdk macosx --show-sdk-path) $(llvm-config --ldflags --libs --system-libs) -o libPass.so

clang -isysroot $(xcrun --show-sdk-path) -Xclang -load -Xclang libPass.so -flegacy-pass-manager main.cpp $(llvm-config --ldflags --libs --system-libs) -lc++
```
