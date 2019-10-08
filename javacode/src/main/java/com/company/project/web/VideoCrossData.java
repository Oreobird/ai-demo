/**
 * Copyright 2017-2025 Evergrande Group.
 */
package com.company.project.web;

import java.io.Serializable;

public class VideoCrossData implements Serializable{

  private static final long serialVersionUID = -8664265816312038999L;
  
  private String videoPathReq;
  private String videoPathRes;
  
  private String progress;
  
  private String errMsg;

  public String getVideoPathReq() {
    return videoPathReq;
  }

  public void setVideoPathReq(String videoPathReq) {
    this.videoPathReq = videoPathReq;
  }

  public String getVideoPathRes() {
    return videoPathRes;
  }

  public void setVideoPathRes(String videoPathRes) {
    this.videoPathRes = videoPathRes;
  }

  public String getProgress() {
    return progress;
  }

  public void setProgress(String progress) {
    this.progress = progress;
  }

  public String getErrMsg() {
    return errMsg;
  }

  public void setErrMsg(String errMsg) {
    this.errMsg = errMsg;
  }

  @Override
  public String toString() {
    return "VideoCrossData [videoPathReq=" + videoPathReq + ", videoPathRes=" + videoPathRes
        + ", progress=" + progress + ", errMsg=" + errMsg + "]";
  }
}
