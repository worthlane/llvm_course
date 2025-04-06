## Установка LLVM

```
export PATH="/opt/homebrew/opt/llvm@14/bin:$PATH"
```

## Сборка пасса

```
clang -c log.c -o log.o -isysroot $(xcrun --show-sdk-path)

clang++ Pass.cpp -c -fPIC -I$(llvm-config --includedir) -isysroot $(xcrun --sdk macosx --show-sdk-path)  -o Pass.o

clang++ Pass.o log.o -fPIC -shared -isysroot $(xcrun --sdk macosx --show-sdk-path) $(llvm-config --ldflags --libs --system-libs) -o libPass.so

clang -isysroot $(xcrun --show-sdk-path) -Xclang -load -Xclang libPass.so -flegacy-pass-manager main.cpp log.o $(llvm-config --ldflags --libs --system-libs) -lc++ >> assets/graph.dot
```

## Генерация графа

```
dot -Tpng assets/graph.dot -o assets/output.png
```
