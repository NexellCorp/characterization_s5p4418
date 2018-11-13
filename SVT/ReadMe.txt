* Directory Architecture
    ┌─ bin      : Application binary folder
    ├─ src      : Source code each module.
    ├─ images   : Second boot/NSIH/U-boot/Kernel Image for SVT.
    ├─ vectors  : Test Vectors.
    ├─ modules  : Module Device Drivers.
    ├─ script   : Rootfile System Scripts for SVT
    └─ fs       : File system for Target


* Build Order
1. Build Boot Loader & copy "u-boot.bin" file to "images" directory.
2. Build Kernel & copy "uImage" file to "images" directory.
3. Build Source : ex> SVT$ make
3. Install Image : ex> SVT$ make install
5. Copy result directory to Target Storage.(Micro SD Card)
  ex > SVT$> cp -a /result/* /media/ray/FAAC-E74B/

* Excute Application
1. Insert Micro SD Card to Micro SD Card Solt in the Target Board.
2. Power On Target Board.
