# CSWBattery
The library for the diy smartwatch projects, which operates with their batteries.
Part of a project which I am working on - the diy smartwatch software, which I am creating for the [LILYGO® TTGO 1.54 Inch Wrist E-paper ESP32 DIY smartwatch](https://www.aliexpress.com/item/1005003095240476.html) with ePaper display.
I think it may be useful for someone else so I make this as a library with the ability to use more than just one button as it is in the case of the mentioned above smartwatch.

And if you'd like to install this software on the same diy smartwatch, please don't forget to follow the manual for the epaper libraries from the [original repo](https://github.com/Xinyuan-LilyGO/E-Paper-watch).


>  - Install the following two dependencies
>    - [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)
>    - please use this branch [lewisxhe/fork/GxEPD](https://github.com/lewisxhe/GxEPD) ,Please note that referencing the wrong branch will cause the program to fail to compile
>  - Copy all the folders in the lib directory to <code>"C:\User\<YourName>\Documents\Arduino\libraries"</code>
>  - At the top of the sketch, define the model of the board and screen to be used
>  - Select <code>ESP32 Dev Module</code> in the development board, and keep the other options as default
>  - If you do not find the ESP32 series in the development board, then you should see the following installation method [How to install ESP32 in ArduinoIDE](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md)
>  - For the version of the board you purchased, please check the product link below
>  - When you think there is a problem with your board, you can pull the binary file corresponding to your screen model in the firmware directory, follow the <code>SETTING.png</code> settings in the directory, download it to the board, and then observe the execution.
>
> -- <cite>[Xinyuan-LilyGO](https://github.com/Xinyuan-LilyGO)</cite>
