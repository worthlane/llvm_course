# LLVM Pass
## Установка LLVM

Работа выполнялась на Apple MacBook Pro M2, для начала нужно было установить сам llvm, я выбрал 14 версию чтобы было комфортнее работать с ```llvm::legacy::PassManager```.
```
brew install llvm@14
```

После установки, нужно настроить окружение.
```
export PATH="/opt/homebrew/opt/llvm@14/bin:$PATH"
export LDFLAGS="-L/opt/homebrew/opt/llvm@14/lib"
export CPPFLAGS="-I/opt/homebrew/opt/llvm@14/include"
```

## Сборка пасса

Сначала мы компилируем вспомогательный файл ```log.c```, содержащий логирующие функции для пасса.
```
clang -c log.c -o log.o -isysroot $(xcrun --show-sdk-path)
```

Затем компилируем основной пасс.
```
clang++ Pass.cpp -c -fPIC -I$(llvm-config --includedir) -isysroot $(xcrun --sdk macosx --show-sdk-path)  -o Pass.o
```

Создаем shared library.
```
clang++ Pass.o log.o -fPIC -shared -isysroot $(xcrun --sdk macosx --show-sdk-path) $(llvm-config --ldflags --libs --system-libs) -o libPass.so
```

Теперь мы можем скомпилировать саму программу, с использованием нашего пасса.
```
clang -isysroot $(xcrun --show-sdk-path) -Xclang -load -Xclang libPass.so -flegacy-pass-manager main.cpp log.o $(llvm-config --ldflags --libs --system-libs) -lc++ >> assets/graph.dot
```

## Генерация графа

Имея dot файл, можем сгенерировать граф в формате png.
```
dot -Tpng assets/graph.dot -o assets/output.png
```

![](/assets/output.png)
