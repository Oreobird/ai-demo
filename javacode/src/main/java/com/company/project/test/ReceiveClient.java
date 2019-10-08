package com.company.project.test;

import java.io.IOException;

import java.io.InputStream;

import java.net.ServerSocket;

import java.net.Socket;

import java.net.SocketException;

import java.util.*;



public class ReceiveClient {

  static int i = 0;

  public static void main(String[] args) throws IOException {

    Timer timer = new Timer();

    timer.schedule(new TimerTask() {

      public void run() {

        i = i + 1;

        if (i == 3) {

          i = 0;

        }

        switch (i) {

          case 1:

            try {

              sockets(6020);

            } catch (SocketException e) {

              // TODO Auto-generated catch block

              e.printStackTrace();

            }
            break;

          case 2:



            try {

              sockets(6019);

            } catch (SocketException e) {

              // TODO Auto-generated catch block

              e.printStackTrace();

            }
            break;

        }

      }

    }, 0, 5000);// 设定指定的时间time,此处为2000毫秒

  }

  public static void sockets(int m) throws SocketException {

    // ，用在字符串缓冲区被单个线程使用的时候（这种情况很普遍）。如果可能，建议优先采用该类，因为在大多数实现中，它比 StringBuffer

    StringBuilder stringBuilder = null;

    stringBuilder = new StringBuilder("");



    // 创建TCP客户端socket服务

    ServerSocket socket = null;

    // 服务端接收客户端数据

    Socket s = null;

    try {

      socket = new ServerSocket(m);// 获取客户端socket对象

      s = socket.accept();// 获取客户端socket输入流对象

      socket.setSoTimeout(5000);

      byte[] buf = new byte[1024];

      InputStream in = null;

      in = s.getInputStream();

      int len = -1;

      len = in.read(buf);

      for (int i = 0; i < len; i++) {

        int v = buf[i] & 0xFF; // 可以使得高位清零

        // 如果integer.tohexstring(buf[i])会导致byte（8位）转化int32位自动补位，结果误差大

        String hv = Integer.toHexString(v);

        if (hv.length() < 2) {

          stringBuilder.append(0);

        }

        stringBuilder.append(hv);

      }

      System.out.println(stringBuilder.toString());

      s.close();

      socket.close();

      // 此方法只有放在in =s.getInputStream();之前才有效，一旦in.read()阻塞就会抛出异常，程序结束！

    } catch (IOException e) {

      // TODO Auto-generated catch block



      try {

        s.close();

        socket.close();

      } catch (IOException e1) {

        // TODO Auto-generated catch block

        e1.printStackTrace();

      }

    }

  }

}

