# coflowSim_online
一个coflow模拟器的在线版本，适用于Varys。


## FB2010-1Hr-150-0.txt Format
```
Line 1: <Number of ports in the fabric> <Number of coflows below (one per line)>
Line i: <Coflow ID> <Arrival time (ms)> <Number of mappers> <Location of map-m> <Number of reducers> <Location of reduce-r:Shuffle megabytes of reduce-r>
```

## machine-define.txt Format
```
Line 1: <Number of machines(one per line)>
Line i: <Machine ID> 
```

## 单机使用docker模拟

1. 在 docker 目录下运行：（ubuntu_coflowsim:1.0 是创建的img的名字及版本号）
```Bash
docker build -t ubuntu_coflowsim:1.0 .
```
2. 运行多台docker容器，例：（其中对于每一台需单独设置的有coflowSim1，4001，172.18.0.1）
```
docker run --name coflowSim1 -p 4001:4003 --net coflowSimNet --ip 172.18.0.1 --mount type=bind,source=/home/he/Git/coflowSim_online/,target=/root/coflowSim_online/,readonly --cap-add=NET_ADMIN -dit ubuntu_coflowsim:1.0
```
3. 注意修改配置文件。