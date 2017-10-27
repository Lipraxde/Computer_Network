NS2模擬分析：
========================================
* 所有TCP Source可用的平寬是平均分配的嗎？
* 長時間傳輸的速度

所有TCP Source可用的平寬是平均分配的嗎
----------------------------------------
簡單的回答：不是
我有設定各個點封包顏色不一樣，左到右分別是：紅、橙、黃、綠、藍
在用nam看模擬出來的結果時就可以發現，藍色的封包特別多，而且可以看到，大部分Queuing的地方在最右邊的Router。

![./window.png](https://github.com/Lipraxde/Computer_Network/blob/master/homework_2/window.png?raw=true)

![./tm0.1\_sw0.5.png](https://github.com/Lipraxde/Computer_Network/blob/master/homework_2/tm0.1_sw0.5.png?raw=true)

我想這是因為Router在傳送封包的時候，是將兩路收到的封包交錯傳送，導致最右邊的TCP Source在傳輸速度上得到比較好的數值。
在模擬過程中看出似乎並非每個TCP Source都像藍色的那個一直傳送封包，是因為距離比較遠的RTT比較久，而且都要經過最後一個Router去列隊，所以傳輸封包的速度就比較慢。
不過似乎並非越遠越慢，像左二橘色竟然是最慢的，可見TCP or FTP在傳送資料上應該有偵測傳輸線路的情況來決定傳輸量。

長時間傳輸的速度
----------------------------------------
拉長傳輸時間後發現，各個TCP Source的傳輸速率起起伏伏的（不過藍色的還是最高），有種週期性的感覺。

模擬時間從10秒拉長到50秒

![./transfer\_time\_1to50.png](https://github.com/Lipraxde/Computer_Network/blob/master/homework_2/transfer_time_1to50.png?raw=true)

累計傳輸量

![./transfer\_data.png](https://github.com/Lipraxde/Computer_Network/blob/master/homework_2/transfer_data.png?raw=true)

可以看到長時間下來還是藍色佔了比較多的頻寬，不過另外四個就比較沒，看還傳輸速率和中間經過幾個節點有很大的關係。看到最後9.9秒的時候總傳輸速度約為

紅(3.44-2.82)/9.9 = 0.0626

橙(2.74-1.83)/9.9 = 0.0919

黃(3.72-2.96)/9.9 = 0.0768

綠(3.97-3.22)/9.9 = 0.0758

藍(10.40-8.50)/9.9 = 0.192

合起來大約0.499Mbit/s，大約0.5Mbit/s。

###### 檔案說明
* hw2.tcl：模擬檔，輸出Y軸速率
* sum\_hw2.tcl：Y軸輸出傳輸資料總和
* window.png：模擬時的截圖
* hw2.nam：送給nam的檔案
* hw2\_0.tr, hw2\_1.tr, hw2\_2.tr, hw2\_3.tr, hw2\_4.tr：輸出的紀錄檔
* plot.gp, sum\_plot.gp：gnuplot的繪圖檔
* througput.png：預設圖片輸出
* tm0.1\_sw0.5.png, tm0.01\_sw0.5.png：之前跑的結果(tm = time move, sw = sliding windows)
* transfer\_data.png：傳輸時間延長到50s的累積傳輸量
* transfer\_time\_1to50.png：傳輸時間延長到50s的Throughput
