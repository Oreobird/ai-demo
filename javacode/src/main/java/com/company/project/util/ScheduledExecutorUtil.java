/**
 * Copyright 2017-2025 Evergrande Group.
 */
package com.company.project.util;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ScheduledExecutorUtil {
  public ScheduledExecutorUtil() {}
  private static final ExecutorService singleScheduledExcutor = Executors.newSingleThreadExecutor();
  public static synchronized void videoCross(Runnable t) {
    singleScheduledExcutor.execute(t);
  }
}
