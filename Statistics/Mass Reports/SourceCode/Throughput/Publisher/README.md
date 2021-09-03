# Fastrtps量測延遲簡易程式碼

## 在執行之前需先確認兩件事

1. `MassReportPublisher.cpp`中的`size`變數需先設定好
2. 再到`MassReport.idl`裡去調整預傳輸的大小

## 編譯方法

```
sh build.sh
```

## 執行方法

```
./MassReportPubExample
```