
# ------- Project for Celoxica : TCP Server -------

Pre-requisites :
Cmake, gcc, telnet installed

Compilation :
1 - In a terminal Navigate to the project directory
2 - mkdir build; cd build
3 - cmake .. (-DENABLE_TIMESTAMP=ON to enable time computation display of GenerateId method)
4 - make 

Execution :
In the build repository :
1 - ./Celo 
2 - on differents terminal : telnet <ipV4 address> 12345
3 - kill the telnet terminals to disconnect clients or Ctrl + C on server





GCC 11.3.0
