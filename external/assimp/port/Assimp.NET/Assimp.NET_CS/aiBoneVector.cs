/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class aiBoneVector : IDisposable, System.Collections.IEnumerable
#if !SWIG_DOTNET_1
    , System.Collections.Generic.IList<aiBone>
#endif
 {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal aiBoneVector(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(aiBoneVector obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~aiBoneVector() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          AssimpPINVOKE.delete_aiBoneVector(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public aiBoneVector(System.Collections.ICollection c) : this() {
    if (c == null)
      throw new ArgumentNullException("c");
    foreach (aiBone element in c) {
      this.Add(element);
    }
  }

  public bool IsFixedSize {
    get {
      return false;
    }
  }

  public bool IsReadOnly {
    get {
      return false;
    }
  }

  public aiBone this[int index]  {
    get {
      return getitem(index);
    }
    set {
      setitem(index, value);
    }
  }

  public int Capacity {
    get {
      return (int)capacity();
    }
    set {
      if (value < size())
        throw new ArgumentOutOfRangeException("Capacity");
      reserve((uint)value);
    }
  }

  public int Count {
    get {
      return (int)size();
    }
  }

  public bool IsSynchronized {
    get {
      return false;
    }
  }

#if SWIG_DOTNET_1
  public void CopyTo(System.Array array)
#else
  public void CopyTo(aiBone[] array)
#endif
  {
    CopyTo(0, array, 0, this.Count);
  }

#if SWIG_DOTNET_1
  public void CopyTo(System.Array array, int arrayIndex)
#else
  public void CopyTo(aiBone[] array, int arrayIndex)
#endif
  {
    CopyTo(0, array, arrayIndex, this.Count);
  }

#if SWIG_DOTNET_1
  public void CopyTo(int index, System.Array array, int arrayIndex, int count)
#else
  public void CopyTo(int index, aiBone[] array, int arrayIndex, int count)
#endif
  {
    if (array == null)
      throw new ArgumentNullException("array");
    if (index < 0)
      throw new ArgumentOutOfRangeException("index", "Value is less than zero");
    if (arrayIndex < 0)
      throw new ArgumentOutOfRangeException("arrayIndex", "Value is less than zero");
    if (count < 0)
      throw new ArgumentOutOfRangeException("count", "Value is less than zero");
    if (array.Rank > 1)
      throw new ArgumentException("Multi dimensional array.", "array");
    if (index+count > this.Count || arrayIndex+count > array.Length)
      throw new ArgumentException("Number of elements to copy is too large.");
    for (int i=0; i<count; i++)
      array.SetValue(getitemcopy(index+i), arrayIndex+i);
  }

#if !SWIG_DOTNET_1
  System.Collections.Generic.IEnumerator<aiBone> System.Collections.Generic.IEnumerable<aiBone>.GetEnumerator() {
    return new aiBoneVectorEnumerator(this);
  }
#endif

  System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() {
    return new aiBoneVectorEnumerator(this);
  }

  public aiBoneVectorEnumerator GetEnumerator() {
    return new aiBoneVectorEnumerator(this);
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class aiBoneVectorEnumerator : System.Collections.IEnumerator
#if !SWIG_DOTNET_1
    , System.Collections.Generic.IEnumerator<aiBone>
#endif
  {
    private aiBoneVector collectionRef;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public aiBoneVectorEnumerator(aiBoneVector collection) {
      collectionRef = collection;
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public aiBone Current {
      get {
        if (currentIndex == -1)
          throw new InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new InvalidOperationException("Collection modified.");
        return (aiBone)currentObject;
      }
    }

    // Type-unsafe IEnumerator.Current
    object System.Collections.IEnumerator.Current {
      get {
        return Current;
      }
    }

    public bool MoveNext() {
      int size = collectionRef.Count;
      bool moveOkay = (currentIndex+1 < size) && (size == currentSize);
      if (moveOkay) {
        currentIndex++;
        currentObject = collectionRef[currentIndex];
      } else {
        currentObject = null;
      }
      return moveOkay;
    }

    public void Reset() {
      currentIndex = -1;
      currentObject = null;
      if (collectionRef.Count != currentSize) {
        throw new InvalidOperationException("Collection modified.");
      }
    }

#if !SWIG_DOTNET_1
    public void Dispose() {
        currentIndex = -1;
        currentObject = null;
    }
#endif
  }

  public void Clear() {
    AssimpPINVOKE.aiBoneVector_Clear(swigCPtr);
  }

  public void Add(aiBone x) {
    AssimpPINVOKE.aiBoneVector_Add(swigCPtr, aiBone.getCPtr(x));
  }

  private uint size() {
    uint ret = AssimpPINVOKE.aiBoneVector_size(swigCPtr);
    return ret;
  }

  private uint capacity() {
    uint ret = AssimpPINVOKE.aiBoneVector_capacity(swigCPtr);
    return ret;
  }

  private void reserve(uint n) {
    AssimpPINVOKE.aiBoneVector_reserve(swigCPtr, n);
  }

  public aiBoneVector() : this(AssimpPINVOKE.new_aiBoneVector__SWIG_0(), true) {
  }

  public aiBoneVector(aiBoneVector other) : this(AssimpPINVOKE.new_aiBoneVector__SWIG_1(aiBoneVector.getCPtr(other)), true) {
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public aiBoneVector(int capacity) : this(AssimpPINVOKE.new_aiBoneVector__SWIG_2(capacity), true) {
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  private aiBone getitemcopy(int index) {
    IntPtr cPtr = AssimpPINVOKE.aiBoneVector_getitemcopy(swigCPtr, index);
    aiBone ret = (cPtr == IntPtr.Zero) ? null : new aiBone(cPtr, false);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private aiBone getitem(int index) {
    IntPtr cPtr = AssimpPINVOKE.aiBoneVector_getitem(swigCPtr, index);
    aiBone ret = (cPtr == IntPtr.Zero) ? null : new aiBone(cPtr, false);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(int index, aiBone val) {
    AssimpPINVOKE.aiBoneVector_setitem(swigCPtr, index, aiBone.getCPtr(val));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void AddRange(aiBoneVector values) {
    AssimpPINVOKE.aiBoneVector_AddRange(swigCPtr, aiBoneVector.getCPtr(values));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public aiBoneVector GetRange(int index, int count) {
    IntPtr cPtr = AssimpPINVOKE.aiBoneVector_GetRange(swigCPtr, index, count);
    aiBoneVector ret = (cPtr == IntPtr.Zero) ? null : new aiBoneVector(cPtr, true);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Insert(int index, aiBone x) {
    AssimpPINVOKE.aiBoneVector_Insert(swigCPtr, index, aiBone.getCPtr(x));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, aiBoneVector values) {
    AssimpPINVOKE.aiBoneVector_InsertRange(swigCPtr, index, aiBoneVector.getCPtr(values));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    AssimpPINVOKE.aiBoneVector_RemoveAt(swigCPtr, index);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    AssimpPINVOKE.aiBoneVector_RemoveRange(swigCPtr, index, count);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public static aiBoneVector Repeat(aiBone value, int count) {
    IntPtr cPtr = AssimpPINVOKE.aiBoneVector_Repeat(aiBone.getCPtr(value), count);
    aiBoneVector ret = (cPtr == IntPtr.Zero) ? null : new aiBoneVector(cPtr, true);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Reverse() {
    AssimpPINVOKE.aiBoneVector_Reverse__SWIG_0(swigCPtr);
  }

  public void Reverse(int index, int count) {
    AssimpPINVOKE.aiBoneVector_Reverse__SWIG_1(swigCPtr, index, count);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetRange(int index, aiBoneVector values) {
    AssimpPINVOKE.aiBoneVector_SetRange(swigCPtr, index, aiBoneVector.getCPtr(values));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool Contains(aiBone value) {
    bool ret = AssimpPINVOKE.aiBoneVector_Contains(swigCPtr, aiBone.getCPtr(value));
    return ret;
  }

  public int IndexOf(aiBone value) {
    int ret = AssimpPINVOKE.aiBoneVector_IndexOf(swigCPtr, aiBone.getCPtr(value));
    return ret;
  }

  public int LastIndexOf(aiBone value) {
    int ret = AssimpPINVOKE.aiBoneVector_LastIndexOf(swigCPtr, aiBone.getCPtr(value));
    return ret;
  }

  public bool Remove(aiBone value) {
    bool ret = AssimpPINVOKE.aiBoneVector_Remove(swigCPtr, aiBone.getCPtr(value));
    return ret;
  }

}
