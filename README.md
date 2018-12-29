# PipeLink
a tiny pipe ipc framework using  iocp in win32 and provider easy use api.  

Src\pipe_imp:  
​	the implement of pipe communication using iocp  

Src\pipe_imp\PipeApi.h:  
​	C style interface  which is a simply warpper of the  CPipeLink Class.       

Src\PipeClient: the demo client source.   
Src\PipeServer: the demo server source.         



	you should call "InitPipe"  when application launch and call"UnInitPipe" when application exit.
