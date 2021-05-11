# **OPC UA 專案作業**

此為配合Festo實驗室的程式設計，為使用OPC UA與機台PLC溝通。如需執行，請先設定好相關環境，步驟如下。

***
環境: ubuntu、cmake、g++

## **安裝步驟**

1. 將整個專案下載成壓縮檔
>- 專案網址: [Open Source C++ OPC-UA Server and Client Library](https://github.com/FreeOpcUa/freeopcua)

2. 上傳壓縮檔至系統
>- 上傳位址: 家目錄即可

3. 解壓縮檔案
>- 解壓縮位址: 家目錄即可
>- 預先下載工具: 如果壓縮檔為zip檔，請先輸入```sudo apt install unzip```指令下載解壓縮工具
>- 解壓縮指令: 如果為zip檔，輸入```unzip XXX.zip```指令

4. 有一個檔案為debian.soft，裡面有專案所需要的工具，必須事先安裝
>- 使用方法: 將裡面的指令複製貼上直接執行即可
>- 指令: ```apt-get install gcc g++ make autoconf automake libtool \
libboost-dev libboost-thread-dev libboost-program-options-dev \
libboost-system-dev libboost-filesystem-dev \
libcppunit-dev pkg-config git python-dev libboost-python-dev \
gsoap libxml2-dev build-essential autotools-dev dh-make \
debhelper devscripts fakeroot xutils lintian pbuilder \
reprepro```
>- 註: 指令中的```\```是換行的意思

5. 執行build.sh
>- 裡面主要是呼叫```autoreconf```指令，如果對於位什麼需要這樣做可以參考此網站: [瞭解automake和autoconf(autoreconf)](https://www.itread01.com/content/1544440350.html)
>- 指令: ```sudo sh build.sh```

6. 下載libmbedtls-dev工具
>- 原由: 到後來執行```make```時，會告知此問題，所以先行下載
>- 指令: ```sudo apt install libmbedtls-dev```

7. 執行cmake指令
>- 作用: 產程**Makefile**檔，讓```make```指令可以執行此檔案
>- 指令: ```cmake .```
>- 註: 請在專案的根目錄底下執行，主要為了執行**CMakeLists.txt**檔

8. 執行make指令
>- 作用: 編譯程式碼成binary file(不會有副檔名，換作是windows環境，則為exe執行檔)
>- 指令: ```make```
>- 註: 請在專案的根目錄底下執行，主要為了執行cmake產生的**Makefile**檔
>- 註: 執行後，binary file會出現在bin資料夾底下

9. 執行binary file
>- 測試是否能建立OPC UA環境
>- 指令: ```./example_server```

## 環境優化
- LD_LIBRARY_PATH的新增
>- 原因: 第一次使用專案時，因為在專案根目錄，許多事情系統已經處理。但當自己新開的資料夾去模擬專案時，會需要一職重複新增 LD_LIBRARY_PATH 位址。
>- 做法: 將專案根目錄中的include資料夾內容```/home/ubuntu/freeopcua-master/include/```複製到```/usr/include```
>- 指令: ```sudo cp -R /home/ubuntu/freeopcua-master/include/* /usr/include```