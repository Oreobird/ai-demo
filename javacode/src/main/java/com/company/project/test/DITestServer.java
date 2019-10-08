/**
 * Copyright 2017-2025 Evergrande Group.
 */
package com.company.project.test;

import java.io.IOException;
import java.io.InputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class DITestServer {
  public static void main(String[] args) throws IOException {
    ServerSocket ss = new ServerSocket(9000);
    try {
      while (true) {
        Socket socket = ss.accept();
        System.out.println("响应客户端：" + System.currentTimeMillis());
        
        InputStream is = socket.getInputStream();
        byte[] data = new byte[8];
        int length;
        while((length = is.read(data)) != -1){
          String result = new String(data);
          System.out.println(result);
          System.out.println("length:" + length);
        }
      }
    } catch (IOException e) {
      System.out.println("HttpServer-->run()方法中出错");
      e.printStackTrace();
    }finally {
      ss.close();
    }
  }
}
