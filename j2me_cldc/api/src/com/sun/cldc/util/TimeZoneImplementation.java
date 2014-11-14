package com.sun.cldc.util;

import java.util.TimeZone;

public abstract class TimeZoneImplementation extends TimeZone {
  public abstract String[] getIDs();
  public abstract TimeZone getInstance(String ID);
}
