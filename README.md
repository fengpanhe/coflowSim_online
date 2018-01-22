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
