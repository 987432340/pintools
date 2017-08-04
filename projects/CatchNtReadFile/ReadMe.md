# 找到拦截的系统调用号

在这里[64位系统调用号](http://j00ru.vexillium.org/ntapi_64/)可以找到 **NtReadFile** 的系统调用号为 0x3 ，这是在代码里硬编码的由来。

# GUI程序无法printf输出 

Pin是在命令行里运行的。pin注入命令行程序可以输出log，注入GUI程序无法输出log，需要用文件输出log