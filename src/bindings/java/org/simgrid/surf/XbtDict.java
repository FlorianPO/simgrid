/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.simgrid.surf;

public class XbtDict {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected XbtDict(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(XbtDict obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        SurfJNI.delete_XbtDict(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public String getValue(String key) {
    return SurfJNI.XbtDict_getValue(swigCPtr, this, key);
  }

  public XbtDict() {
    this(SurfJNI.new_XbtDict(), true);
  }

}
