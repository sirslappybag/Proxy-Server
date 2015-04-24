# Proxy-Server
Group 13 - Proxy 

In this project, we are developing a web proxy. The proxy should be able to handle a HTTP request and consequent sequence of events: send new request to host server, handle response and forward response back to the browser. In addition, we were required to implement a cache and filter. The filter will block web sites if they are found in our black list and it will filter out any inappropriate language. 

To compile: gcc proxy.c -lpthread
Run with ./a.out

To access you need to run from http://129.120.151.98:8884/[whatever website]
