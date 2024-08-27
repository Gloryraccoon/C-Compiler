编译平台:
ubuntu22.04.4lts server
在项目目录使用cmake命令即可
或者vscode连接虚拟机，使用生成可执行文件按钮即可

使用方式:
./minic -S -a -o [输出文件名] 源文件名   =>生成ast图
./minic -S -i/-I -o [输出文件名] 源文件名  =>生成中间ir序列
