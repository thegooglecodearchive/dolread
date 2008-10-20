注意: 运行测试文件前请先备份卡上数据!

如不是第一次运行此测试, 请先删除在测试中生成的fat_test文件夹, 然后再运行测试.
如果第一次运行此测试, 请将test_files.zip解压后把文件拷入SD/TF卡根目录

下列烧录卡使用clean rom(FAT_Test.nds):
SCDS
NC

下列文件直接拷入对应烧录卡运行即可:
AK
DSLink
Ewin2
EZ5
M3_Simply(使用R4的文件)
M3_S2(S2=SLOT2)
G6_S2


测试运行时间约30-900s(依赖于存储卡的读写速度), 请耐心等待
提示Test Passed和Test Failed后, 记录下测试结果后(图片最好)关机即可
谢谢 :)


测试案例:
1. 简单文件读写
过程: 写入一个简单字符串"Just writing text to a file :)"到文件"FATTest.txt", 然后读取文件"FATTest.txt"的内容, 检查是否和写入的字符串一致.

2. 大文件读写
过程: 写入5MB的数据到文件"FATTest.dat", 然后读取文件"FATTest.dat"的内容, 检查是否和写入的数据一致.

3. 大块读取
过程: 用256KB的buffer循环读取"FATTest.dat"的5MB数据, 检查是否20次读完. 该案例依赖测试案例#2.

4. 目录操作测试, 测试了基本的目录操作函数: chdir, diropen, dirnext, dirreset, dirclose
过程: 略, 详见代码

5. 复制文件
过程: 在根目录写入文件"FATTest1.dat", 并复制到"FATTest2.dat"

6. 移动游标读写
过程: 按"1234"序列循环写入4MB数据到文件"FATTest.dat", 并随机移动游标(当前版本游标位置4字节对齐)读取数据, 检查在相应位置上读出的数据是否正确.

7. 长文件名操作
过程: 测试是否能进入目录名为中文的目录和打开文件名为中文的文件
