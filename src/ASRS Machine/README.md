此目錄(模組根目錄)的所有資料夾，皆為分類使用，並無法直接使用`cmake .  && make`編譯完成。如需使用，編譯前，須將所有子目錄的資料移至模組根目錄。在進行編譯相關動作。

編譯指令虛擬碼:
```
fastrtpsgen {eachIdlFile}.idl

cmake .

make
```