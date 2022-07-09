# My progress for labs of CS:APP3e

- [x] Lab1: data lab
- [x] Lab2: bomb lab
- [x] Lab3: attack lab
- [x] Lab4: cache lab
- [x] Lab5: shell lab
- [x] Lab6: malloc lab
- [ ] Lab7: proxy lab

## install
docker get centos image:
```
docker pull centos
```

create a container:
```
docker container run -it -v /Users/xxxx/yourFilePath:/csapp --name=csapp_env centos /bin/bash
```
/Users/xxxx/yourFilePath : the syn path of your computer dir;

Build the environment:
```
yum -y update
yum install sudo
yum install make automake gcc gcc-c++ kernel-devel
yum install gdb
yum install glibc-devel.i686
```

enter the container later:
```
docker exec -it XXX(your container ID) /bin/bash
```


