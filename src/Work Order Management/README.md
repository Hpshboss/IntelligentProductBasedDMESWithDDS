此目錄(模組根目錄)的所有資料夾，皆為分類使用，並無法直接使用`cmake .  && make`編譯完成。如需使用，編譯前，須將所有子目錄的資料移至模組根目錄。在進行編譯相關動作。

**編譯指令虛擬碼:**
```
fastrtpsgen {eachIdlFile}.idl

cmake .

make
```

**Fix Record**

- **20210614**
>- 完成系統邏輯撰寫，且編譯成功，但尚未測試
>- 修正Recipe Info的資料結構，增加GUID
>- Recipe Res概念擴增，產品廣播