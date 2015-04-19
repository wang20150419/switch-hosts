# switch-hosts
# auto switch hosts file by router mac address
 
 
SWhosts用法(v2015.04.19)：

(操作系统要求：windows 7/8/8.1)

根据网关MAC地址，自动选用对应的HOSTS文件。

1）hosts.default为缺省hosts文件，当条件不匹配的时候使用。

2）swhosts.ini 为配置文件，格式为： 网关MAC地址 空格 对应HOSTS文件。

3）网关MAC地址查询: 在控制台窗口下，运行 arp -a，看网关地址对应的那行。

4）自动切换方法：

   启动事件查看器，选应用程序和服务日志，选Microsoft-Windows-NetworkProfile/Operational，
   
   对 event ID 为 10000 的事件，配置任务（右边，“将任务附加到此事件...”）,对应的操作为
   
   执行程序 swhosts.exe，对64bit系统，使用swhosts_x64.exe。
   

5）可到“任务计划程序”的“事件查看器任务”里，调整swhosts的相关属性。

6) 应用场景：

    办公室有ipv6地址，通过特定的ipv6 hosts文件，可以访问google,gmail,youtube等。
    离开办公室的时候，使用ipv4 hosts文件，通过穿墙等方法访问google,gmail...。
    利用此程序，可以根据场景自动切换hosts文件。
    
7) 执行程序下载：
     
     64bit: https://drive.google.com/file/d/0BzoaUWj_3m8tbS1GZm9LRGFCYkE/view?usp=sharing
     
     32bit: https://drive.google.com/file/d/0BzoaUWj_3m8tNnFJWklOX0J3TWs/view?usp=sharing
     
8) ipv6 hosts:
     https://github.com/lennylxx/ipv6-hosts
     
