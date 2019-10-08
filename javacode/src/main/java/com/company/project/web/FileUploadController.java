package com.company.project.web;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.util.Base64;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.company.project.core.Result;
import com.company.project.core.ResultGenerator;
import com.company.project.util.NumberUtil;
import com.company.project.util.ScheduledExecutorUtil;

@RestController
@RequestMapping("/file")
public class FileUploadController {

  @Value("${upload.img-path}")
  private String imgPath;
  @Value("${upload.video-path}")
  private String videoPath;

  @RequestMapping("/test")
  public String test() {
    return "this is a test!";
  }

  private String _readToString(String fileName) {
    String encoding = "UTF-8";
    File file = new File(fileName);
    Long filelength = file.length();
    byte[] filecontent = new byte[filelength.intValue()];
    try {
      FileInputStream in = new FileInputStream(file);
      in.read(filecontent);
      in.close();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    }
    try {
      return new String(filecontent, encoding);
    } catch (UnsupportedEncodingException e) {
      System.err.println("The OS does not support " + encoding);
      e.printStackTrace();
      return null;
    }
  }

  /**
   * 转发请求给AI程序处理
   *
   * @param fileContent
   * @return string
   * @throws Exception
   */
  private String tcpToAi(byte[] fileContent, Boolean isnoEncode, int aiPort) throws Exception {
    Socket client = new Socket(aiHost, aiPort);
    OutputStream outputStream = client.getOutputStream();
    DataOutputStream out = new DataOutputStream(outputStream);
    byte[] encodeContent = null;
    if (isnoEncode != null && isnoEncode) {
      encodeContent = fileContent;
      System.out.println("\n4:" + encodeContent.length);
      printHexString(encodeContent);
      encodeContent =
          NumberUtil.unitByteArray(NumberUtil.intToByte4(encodeContent.length), encodeContent);
      System.out.println("\n5:" + encodeContent.length);
      printHexString(encodeContent);
      out.write(encodeContent);
    } else {
      encodeContent = Base64.getEncoder().encode(fileContent);
      System.out.println("\n4:" + encodeContent.length);
      printHexString(encodeContent);
      encodeContent =
          NumberUtil.unitByteArray(NumberUtil.intToByte4(encodeContent.length), encodeContent);
      System.out.println("\n5:" + encodeContent.length);
      printHexString(encodeContent);
      out.write(encodeContent);
    }
    System.out.println(encodeContent);
    InputStream inputStream = client.getInputStream();
    DataInputStream in = new DataInputStream(inputStream);

    byte[] lengthByte = new byte[4];
    for (int i = 0; i < 4; i++) {
      lengthByte[i] = in.readByte();
    }
    int length = NumberUtil.byte4ToInt(lengthByte, 0);
    System.out.println("\n" + length);

    byte[] data = new byte[length];
    in.readFully(data);
    client.close();

    String res = new String(data, "UTF-8");
    System.out.println(res);
    return res;
  }

  public static void printHexString(byte[] b) {
    for (int i = 0; i < b.length; i++) {
      String hex = Integer.toHexString(b[i] & 0xFF);
      if (hex.length() == 1) {
        // hex = '0' + hex;
      }
      System.out.print(hex.toUpperCase());
    }

  }

  /**
   * 人脸质量检测
   */
  @PostMapping("/img/upload")
  public Result uploadImg(@RequestParam("filename") MultipartFile file,
      @RequestParam("algorithm") Integer algorithm) throws Exception {
    String fileType = file.getContentType();
    if (!fileType.equals("image/jpeg") && !fileType.equals("image/png")) {
      return ResultGenerator.genFailResult("请上传jpg或者png图片");
    }

    String data = this.tcpToAi(file.getBytes(), null, 6666);
    JSONObject object = JSON.parseObject(data);

    Map res = new HashMap();
    res.put("result", object);
    return ResultGenerator.genSuccessResult(res);
  }

  /**
   * 绊线检查
   */
  @PostMapping("/img2/upload")
  public Result uploadImg2(@RequestParam("img_data") MultipartFile file,
      @RequestParam("line_points") String linePoints) throws Exception {
    String fileType = file.getContentType();
    if (!fileType.equals("image/jpeg") && !fileType.equals("image/png")) {
      return ResultGenerator.genFailResult("请上传jpg或者png图片");
    }

    byte[] encodeContent = file.getBytes();
    System.out.println("1:" + encodeContent.length);
    // encodeContent = NumberUtil.unitByteArray(NumberUtil.intToByte4(encodeContent.length),
    // encodeContent);
    System.out.println("2:" + encodeContent.length);
    StringBuilder sb = new StringBuilder();
    sb.append("{\"img_data\":\"").append(Base64.getEncoder().encodeToString(encodeContent))
        .append("\",");
    sb.append("\"line_points\":").append(linePoints).append("}");
    System.out.println("3:" + sb.length());
    String data = this.tcpToAi(sb.toString().getBytes(), true, 6676);
    JSONObject object = JSON.parseObject(data);

    Map res = new HashMap();
    res.put("result", object);
    return ResultGenerator.genSuccessResult(res);
  }

  /**
   * 人脸识别
   */
  @PostMapping("/img3/upload")
  public Result uploadImg3(@RequestParam("filename") MultipartFile file,
      @RequestParam("type") String type, String name, HttpServletRequest request) throws Exception {
    System.out.println(request.getContentType());
    System.out.println(file);
    String fileType = file.getContentType();
    if (!fileType.equals("image/jpeg") && !fileType.equals("image/png")) {
      return ResultGenerator.genFailResult("请上传jpg或者png图片");
    }

    byte[] encodeContent = file.getBytes();
    System.out.println("1:" + encodeContent.length);
    // encodeContent = NumberUtil.unitByteArray(NumberUtil.intToByte4(encodeContent.length),
    // encodeContent);
    System.out.println("2:" + encodeContent.length);
    StringBuilder sb = new StringBuilder();
    sb.append("{\"type\":").append(type).append(",");
    if (StringUtils.isNotBlank(name)) {
      sb.append("\"name\":\"").append(name).append("\",");
    }
    sb.append("\"img_data\":\"").append(Base64.getEncoder().encodeToString(encodeContent))
        .append("\"}");
    System.out.println("3:" + sb.length());
    System.out.println("to:=====>" + sb.toString());
    String data = this.tcpToAi(sb.toString().getBytes(), true, 6686);
    JSONObject object = JSON.parseObject(data);

    Map res = new HashMap();
    res.put("result", object);
    return ResultGenerator.genSuccessResult(res);
  }


  /**
   * 人脸识别
   */
  @PostMapping("/img4/upload")
  public Result uploadImg4(@RequestParam("filename") MultipartFile file,

      HttpServletRequest request) throws Exception {
    System.out.println(request.getContentType());
    System.out.println(file);
    String fileType = file.getContentType();
    if (!fileType.equals("image/jpeg") && !fileType.equals("image/png")) {
      return ResultGenerator.genFailResult("请上传jpg或者png图片");
    }

    String data = this.tcpToAi(file.getBytes(), null, 6686);
    JSONObject object = JSON.parseObject(data);

    Map res = new HashMap();
    res.put("result", object);
    return ResultGenerator.genSuccessResult(res);
  }

  /**
   * 视频绊线检查 6696
   */
  @PostMapping("/video/upload")
  public Result uploadVideo(@RequestParam("video_data") MultipartFile file,
      @RequestParam("line_points") String linePoints) throws Exception {

    String pathfile = videoPath +new Date().getTime()+"_"+file.getOriginalFilename();
     file.transferTo(new File(pathfile));
System.out.println(pathfile);
    StringBuilder sb = new StringBuilder();
    sb.append("{\"video_path\":\"").append(pathfile).append("\",");
    sb.append("\"line_points\":").append(linePoints).append("}");
    System.out.println("req: " + sb.toString());
    ScheduledExecutorUtil.videoCross(new Runnable() {
      @Override
      public void run() {
        try(Socket client = new Socket(aiHost, 6696)) {
          OutputStream outputStream = client.getOutputStream();
          DataOutputStream out = new DataOutputStream(outputStream);
          byte[] encodeContent = sb.toString().getBytes();;
          printHexString(encodeContent);
          encodeContent =
              NumberUtil.unitByteArray(NumberUtil.intToByte4(encodeContent.length), encodeContent);
          printHexString(encodeContent);
          out.write(encodeContent);
          System.out.println(encodeContent);
          InputStream inputStream = client.getInputStream();
          DataInputStream in = new DataInputStream(inputStream);
          
          while(true) {
            byte[] lengthByte = new byte[4];
            for (int i = 0; i < 4; i++) {
              lengthByte[i] = in.readByte();
            }
            int length = NumberUtil.byte4ToInt(lengthByte, 0);
            System.out.println("\n" + length);
    
            byte[] data = new byte[length];
            in.readFully(data);
            
            String res = new String(data, "UTF-8");
            System.out.println(res);
            
            if(res.indexOf("\"err_code\":0") < 0) {
              videoProgress.get(pathfile).setErrMsg(res.substring(res.indexOf("\"err_msg\":")+11,res.lastIndexOf("}")-1));
              break;
            }
            if(res.indexOf("video_path") >= 0) {
              videoProgress.get(pathfile).setProgress("100%");
              videoProgress.get(pathfile).setVideoPathRes(res.substring(res.indexOf("\"video_path\":")+14,res.lastIndexOf("}")-1));
              break;
            }
            
            int idx = res.indexOf("\"progress\":");
            if(idx >= 0) {              
              videoProgress.get(pathfile).setProgress(res.substring(idx+12, res.lastIndexOf("%")+1));
            }
          }
            
        }catch(Exception e){
          System.out.println("error:"+e.getMessage());
          System.out.println(e);
          videoProgress.get(pathfile).setErrMsg(e.getMessage());
        }
      }
    });
   
    VideoCrossData vcd = new VideoCrossData();
    vcd.setVideoPathReq(pathfile);
    vcd.setProgress("0%");
    videoProgress.put(pathfile, vcd);
    
    Thread.sleep(3000);
    
    String data = null;
    if("100%".equals(vcd.getProgress()) && StringUtils.isNotBlank(vcd.getVideoPathRes())) {
      data = "{\"err_code\":0, \"video_path\":\""+vcd.getVideoPathRes()+"\"}";
      videoProgress.remove(pathfile);
    }else {
      data = "{\"err_code\":0, \"progress\":\""+vcd.getProgress()+"\",\"video_source\":\""+pathfile+"\"}";
    }
    JSONObject object = JSON.parseObject(data);

    Map res = new HashMap();
    res.put("result", object);
    return ResultGenerator.genSuccessResult(res);
  }
  
  @GetMapping("/video/progress")
  public Result videoProgress(@RequestParam("video_source") String pathfile) throws Exception {
    System.out.println(videoProgress);
    VideoCrossData vcd = videoProgress.get(pathfile);
   
    String data = null;
    if(vcd == null) {
      data = "{\"err_code\":-1, \"err_msg\":\"video source file no exist cache.\"}";
    }else {
      System.out.println(vcd);
      if(StringUtils.isNotBlank(vcd.getErrMsg())) {
        data = "{\"err_code\":-1, \"err_msg\":\""+vcd.getErrMsg()+"\"}";
      }else if("100%".equals(vcd.getProgress()) && StringUtils.isNotBlank(vcd.getVideoPathRes())) {
        data = "{\"err_code\":0, \"video_path\":\""+vcd.getVideoPathRes()+"\"}";
        videoProgress.remove(pathfile);
      }else {
        data = "{\"err_code\":0, \"progress\":\""+vcd.getProgress()+"\"}";
      }
    }
    System.out.println(data);
    JSONObject object = JSON.parseObject(data);
    Map res = new HashMap();
    res.put("result", object);
    return ResultGenerator.genSuccessResult(res);
  }
public static void main(String[] args) {
  String res = "{\"err_code\":0, \"progress\":\"85%\"}";
  System.out.println(res.substring(res.indexOf("\"progress\":")+12, res.lastIndexOf("%")+1));
}
  @Value("${ai.server.host:127.0.0.1}")
  private String aiHost;
  private static Map<String,VideoCrossData> videoProgress = new HashMap<>();
}
