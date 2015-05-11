# Introduction #

本页介绍如何build Dolphin Reader的rom以及.dr文件转换器rec\_conv


# Details #

Dolphin Reader是一款基于devkitpro和Palib开发的NDS应用程序
请先安装devkitpro\_r20和palib070615

另外还需安装python和mingw, 版本无特殊要求

# Patches #

给palib打补丁，修正了timer和装载bitmap文件的问题:
  * 将patch/palib070615目录下的覆盖原有代码
  * 重新编译palib

给libfat打补丁，修正了在fat16系统中可能出现的访问越界问题，并加入了对Unicode的支持:
  * 从devkitpro的仓库中取得日期为20070902的libfat的代码
  * 将patch/palib070615目录下的同名文件覆盖原有代码，然后重新编译libfat
  * 只编译针对NDS的版本即可: make -C nds BUILD=release
  * 注意：一定要将新生成的libfat.a复制到"X:devkitPro\libnds\lib"路径下覆盖原有文件

生成libutf.a:
  * 从 http://www.unicode.org/Public/BETA/CVTUTF-1-4/ 取得Unicode的转换代码
  * 使用下列命令生成libutf.a:
    * arm-eabi-gcc -c ConvertUTF.c
    * arm-eabi-ar -rcs libutf.a ConvertUTF.o
  * 将生成的libutf.a文件复制到"X:devkitPro\libnds\lib"

# Build #

进入dsbuild目录, 执行build.py即可生成Dolphin Reader的clean rom

进入rec\_conv目录， 执行make即可生成.dr文件转换器rec\_conv.exe