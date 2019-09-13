## shttpd v1.0.0 介绍

shttpd 为静态web 服务器，借鉴libevent的核心思想、采用EPOLL反应堆模型，目前为开发版本，仅供测试、学习用。
目前由于该项目仅作为本人学习用，暂时还没有提供日志功能，也没有提供CGI功能，http协议也做的比较粗放，后续有时间会补上。
最后感谢万能的互联网，项目中字符串处理函数借鉴于此
（泪目～，我为啥不安安静静地直接用nginx就好了呢...，好奇心这东西一碰就再也收不回了，http协议、FastCGI协议。。。感觉下一步是不是准备写一个sphp出来了）


## 安装

执行 `make` 命令即可，最终生成的可执行文件为bin目录下的shttpd


## 使用

执行 `./bin/shttpd -d web根目录` 
参数选项：
 - `-d `: 必须选项，指定web根目录
 - `-a` : 绑定IP地址，默认为127.0.0.1
 - `-p` : 绑定端口，默认为8888
 - `-v` : 查看版本号
 - `-h或-?` :  查看帮助



<video id="video" controls="" preload="none" >
      <source id="mp4" src="shttpd_webserver.mp4" type="video/mp4">     
      <p>Your user agent does not support the HTML5 Video element.</p>
</video>




