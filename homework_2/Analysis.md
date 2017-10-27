NS2模擬分析：
========================================
* 所有TCP Source可用的平寬是平均分配的嗎？
* 拉長傳輸時間呢？

所有TCP Source可用的平寬是平均分配的嗎
----------------------------------------
簡單的回答：不是
我有設定各個點封包顏色不一樣，左到右分別是：紅、橙、黃、綠、藍
在用nam看模擬出來的結果時就可以發現，藍色的封包特別多，而且可以看到，大部分Queuing的地方在最右邊的Router。

![模擬圖](https://github.com/Lipraxde/Computer_Network/blob/master/homework_2/window.png?raw=true)

![Throughput](https://github.com/Lipraxde/Computer_Network/blob/master/homework_2/tm0.1_sw0.5.png?raw=true)

我想這是因為Router在傳送封包的時候，是將兩路收到的封包交錯傳送，導致越左邊的權重越低。
不過在模擬過程中看出似乎並非每個TCP Source都像藍色的那個一直傳送封包，是因為距離比較遠的RTT比較久，所以傳輸封包的次數就比較少吧。

拉長傳輸時間呢
----------------------------------------
拉長傳輸時間後發現，各個TCP Source的傳輸速率起起伏伏的（不過藍色的還是最高），有種週期性的感覺。

![模擬時間從10秒拉長到50秒](https://github.com/Lipraxde/Computer_Network/blob/master/homework_2/transfer_time_1to50.png?raw=true)

累計傳輸量
![模擬時間從10秒拉長到50秒](https://github.com/Lipraxde/Computer_Network/blob/master/homework_2/transfer_data.png?raw=true)

可以看到長時間下來還是藍色佔了比較多的頻寬，不過另外四個就比較沒，看還傳輸速率和中間經過幾個節點有很大的關係（為什麼會成階梯形呢？因為對TCL不熟，用到整數運了）看到50秒的時候總傳輸速度約為(10+3+3+3+2)/50=0.42，沒超過0.5Mbit/s。
