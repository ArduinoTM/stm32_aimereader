# stm32_aimereader
aime读卡器新方案
<h1> 我去，我今天才发现怎么开了这个页面但是没写东西！！！ </h1>
即将更新  

# 总览

文章正在逐步编写，暂时完成一部分

上传的程序是对Sucareto/Arduino-Aime-Reader程序的修改版本，适用于stm32duino，由于stm32duino使用FastLED报错，所以改为Adafruit-Neopixel库，暂发现一定情况下与原程序不一致（？，但影响非常非常非常小，几乎没有，几乎可以说一点也不影响使用

使用教程后续会发，现在简述，首先安装arduino ide，然后安装开发板stm32duino，然后安装pn532库和neopixel库。开发板选择F103 ， 具体选BluePill F103C8T6，因为stm32duino里这个可以跑USB，好像标准C8T6那个stm32duino不大支持。U(S)ART选Enabled(Generic Serial)，USB模式选CDC (Generic Serial supersede U(S)ART)，USB速度Low/Full，吸不吸氧(优化)，氧气（-O2）还是臭氧(-O3)随意，因为程序对速度没有要求，选择不优化已经足够可以，然后选择合适下载器编译下载即可。（如果你没有列表支持下载器，请在项目栏选择导出编译好的二进制，hex/bin文件将会生成在项目文件夹，使用其他软硬件比如J-flash，jlink下载即可），程序默认LED在PA6，pn532接在第一个I2C即可。（烧录完成请重新拔插设备以使usb生效！）

本文章方案内容和其中所述设备为使用 Sucareto/Arduino-Aime-Reader 程序的 Aime 兼容读卡器

**注意！本方案的测试作品还在制作中，基本软件程序已经开发测试完成，PCB的MCU主控部分和灯都正常工作，但是532部分暂未调起来！！！不要直接打板！！本文章暂且只做方案展示，不作教程，后续测试V1Alpha成功后，将会发布V1正式版PCB文件和教程！！！！**

以下是本方案特色：  

1. 使用 STM32F103C8T6 作为主控芯片，此芯片支持USB Device-FS，可以直接作为一个 USB-CDC 设备接入计算机，并在计算机上显示为一个 USB 串行设备（串口），免去了CH340一类USB转串口芯片的成本，且不用Atmega32U4的高价格，做到了低价（每片5-10元）同时，也带来一些独特的优势（下面会讲到）。
2. 使用 USB-CDC 的优势： a. 免驱动，即插即用，无需像ch340一样安装驱动。b.不经过实际的"TTL"，"RS-232"环节，无需波特率通信速度设定，所以可以兼容在任意波特率下免设置即使用，免去了切换38400和115200的麻烦。（原理下面讲述）
3. 所有元件全部贴片于一张PCB，整个PCB浑然一体，减少了厚度。无需插模块，也避免了模块卖家的利润对读卡器的成本造成增加。由于pn532也做在pcb上，可以借用读卡器较大的面积，铺设更大的天线，做好天线匹配应该能做出比小模块更好的读取效果。
4. 我将所有元件设计于同一面，LED使用元件面反贴方式沉板，使整个读卡器正面是一整个平面，白色的阻焊非常美观，而且方便装配（贴）亚克力片或其他片状物体装饰。
5. 由于屏幕确实没有很大必要，且造成十多元成本，所以将屏幕去除做到正面平整。但是为了满足更多人需求，我将线圈从V0.1内部测试失败版的中间向下挪，在pcb上方留出空隙，并做了I2C OLED所用的四个开窗焊点，这样可以一个板子实现加与不加屏两种型号，而且小的焊盘基本不影响正面外观。焊点也有考量，选择屏幕居中放置时屏幕遮挡的位置，这样在安装屏幕后将隐藏连线。

