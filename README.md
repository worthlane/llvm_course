# LLVM Pass
## Структура пасса
## Конфигурация
### Установка LLVM

Работа выполнялась на Apple MacBook Pro M2, для начала нужно было установить сам llvm, я выбрал 14 версию чтобы было комфортнее работать с ```llvm::legacy::PassManager```.
```
brew install llvm@14
```

После установки нужно настроить окружение.
```
export PATH="/opt/homebrew/opt/llvm@14/bin:$PATH"
export LDFLAGS="-L/opt/homebrew/opt/llvm@14/lib"
export CPPFLAGS="-I/opt/homebrew/opt/llvm@14/include"
```

### Сборка пасса

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
clang -isysroot $(xcrun --show-sdk-path) -Xclang -load -Xclang libPass.so -flegacy-pass-manager main.cpp log.o $(llvm-config --ldflags --libs --system-libs) -lc++
```

### Генерация графа

Имея dot файл, можем сгенерировать граф в формате png.
```
dot -Tpng [dot_file] -o [resulted_png]
```

Имея динамическую информацию по инструкциям, после исполнения программы, можем получить раскрашенный def-use граф. Раскрашен по принципу: Более красные вершины исполнялись чаще, чем более зеленые. Получить данный граф можем с помощью python скрипта.

```
python3 combine.py
```

## Пример работы

Проверим всё на простой программе, например:
```cpp
int main() {
  int a = 0;
  int b = 1;

  a = ++a - b;

  return --a;
}
```

Полученный def-use граф будет выглядеть примерно так:

![](/readme_assets/simple/output.png)

Используем python скрипт, чтобы раскрасить вершины

![](/readme_assets/simple/dynamic_output.png)

Как можно заметить, все вершины, связанные с инструкциями, окрашены в красный. Это связано с тем, что все инструкции были выполнены одинаковое количество раз.

Попробуем на другом примере:

```cpp
int main() {
  int a = 0;
  a += 2;

  int b = 100;
  while (--b) {
  }

  int c = 40;
  while (--c) {
  }

  return 0;
}
```

Результатом будет граф

![](/readme_assets/cycle/dynamic_output.png)

Здесь мы уже можем заметить более разнообразные цвета. Так как циклы в примере исполняются за разное количество операций, некоторые вершины в графе более красные, чем другие.

В папке [examples](examples) можно найти примеры работы пасса на тяжеловесных программах (я брал сложновычислимые задачи из контеста). Так как в ридми подобные картинки будет очень сложно разглядеть.

Например, один из примеров выглядит как то так:
![](/readme_assets/heavy_result.png)



