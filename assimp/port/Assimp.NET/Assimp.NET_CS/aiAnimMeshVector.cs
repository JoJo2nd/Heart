/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class aiAnimMeshVector : IDisposable, System.Collections.IEnumerable
#if !SWIG_DOTNET_1
    , System.Collections.Generic.IList<aiAnimMesh>
#endif
 {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal aiAnimMeshVector(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(aiAnimMeshVector obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~aiAnimMeshVector() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          AssimpPINVOKE.delete_aiAnimMeshVector(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public aiAnimMeshVector(System.Collections.ICollection c) : this() {
    if (c == null)
      throw new ArgumentNullException("c");
    foreach (aiAnimMesh element in c) {
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

  public aiAnimMesh this[int index]  {
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
  public void CopyTo(aiAnimMesh[] array)
#endif
  {
    CopyTo(0, array, 0, this.Count);
  }

#if SWIG_DOTNET_1
  public void CopyTo(System.Array array, int arrayIndex)
#else
  public void CopyTo(aiAnimMesh[] array, int arrayIndex)
#endif
  {
    CopyTo(0, array, arrayIndex, this.Count);
  }

#if SWIG_DOTNET_1
  public void CopyTo(int index, System.Array array, int arrayIndex, int count)
#else
  public void CopyTo(int index, aiAnimMesh[] array, int arrayIndex, int count)
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
  System.Collections.Generic.IEnumerator<aiAnimMesh> System.Collections.Generic.IEnumerable<aiAnimMesh>.GetEnumerator() {
    return new aiAnimMeshVectorEnumerator(this);
  }
#endif

  System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() {
    return new aiAnimMeshVectorEnumerator(this);
  }

  public aiAnimMeshVectorEnumerator GetEnumerator() {
    return new aiAnimMeshVectorEnumerator(this);
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class aiAnimMeshVectorEnumerator : System.Collections.IEnumerator
#if !SWIG_DOTNET_1
    , System.Collections.Generic.IEnumerator<aiAnimMesh>
#endif
  {
    private aiAnimMeshVector collectionRef;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public aiAnimMeshVectorEnumerator(aiAnimMeshVector collection) {
      collectionRef = collection;
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public aiAnimMesh Current {
      get {
        if (currentIndex == -1)
          throw new InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new InvalidOperationException("Collection modified.");
        return (aiAnimMesh)currentObject;
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
    AssimpPINVOKE.aiAnimMeshVector_Clear(swigCPtr);
  }

  public void Add(aiAnimMesh x) {
    AssimpPINVOKE.aiAnimMeshVector_Add(swigCPtr, aiAnimMesh.getCPtr(x));
  }

  private uint size() {
    uint ret = AssimpPINVOKE.aiAnimMeshVector_size(swigCPtr);
    return ret;
  }

  private uint capacity() {
    uint ret = AssimpPINVOKE.aiAnimMeshVector_capacity(swigCPtr);
    return ret;
  }

  private void reserve(uint n) {
    AssimpPINVOKE.aiAnimMeshVector_reserve(swigCPtr, n);
  }

  public aiAnimMeshVector() : this(AssimpPINVOKE.new_aiAnimMeshVector__SWIG_0(), true) {
  }

  public aiAnimMeshVector(aiAnimMeshVector other) : this(AssimpPINVOKE.new_aiAnimMeshVector__SWIG_1(aiAnimMeshVector.getCPtr(other)), true) {
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public aiAnimMeshVector(int capacity) : this(AssimpPINVOKE.new_aiAnimMeshVector__SWIG_2(capacity), true) {
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  private aiAnimMesh getitemcopy(int index) {
    IntPtr cPtr = AssimpPINVOKE.aiAnimMeshVector_getitemcopy(swigCPtr, index);
    aiAnimMesh ret = (cPtr == IntPtr.Zero) ? null : new aiAnimMesh(cPtr, false);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private aiAnimMesh getitem(int index) {
    IntPtr cPtr = AssimpPINVOKE.aiAnimMeshVector_getitem(swigCPtr, index);
    aiAnimMesh ret = (cPtr == IntPtr.Zero) ? null : new aiAnimMesh(cPtr, false);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(int index, aiAnimMesh val) {
    AssimpPINVOKE.aiAnimMeshVector_setitem(swigCPtr, index, aiAnimMesh.getCPtr(val));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void AddRange(aiAnimMeshVector values) {
    AssimpPINVOKE.aiAnimMeshVector_AddRange(swigCPtr, aiAnimMeshVector.getCPtr(values));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public aiAnimMeshVector GetRange(int index, int count) {
    IntPtr cPtr = AssimpPINVOKE.aiAnimMeshVector_GetRange(swigCPtr, index, count);
    aiAnimMeshVector ret = (cPtr == IntPtr.Zero) ? null : new aiAnimMeshVector(cPtr, true);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Insert(int index, aiAnimMesh x) {
    AssimpPINVOKE.aiAnimMeshVector_Insert(swigCPtr, index, aiAnimMesh.getCPtr(x));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, aiAnimMeshVector values) {
    AssimpPINVOKE.aiAnimMeshVector_InsertRange(swigCPtr, index, aiAnimMeshVector.getCPtr(values));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    AssimpPINVOKE.aiAnimMeshVector_RemoveAt(swigCPtr, index);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    AssimpPINVOKE.aiAnimMeshVector_RemoveRange(swigCPtr, index, count);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public static aiAnimMeshVector Repeat(aiAnimMesh value, int count) {
    IntPtr cPtr = AssimpPINVOKE.aiAnimMeshVector_Repeat(aiAnimMesh.getCPtr(value), count);
    aiAnimMeshVector ret = (cPtr == IntPtr.Zero) ? null : new aiAnimMeshVector(cPtr, true);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Reverse() {
    AssimpPINVOKE.aiAnimMeshVector_Reverse__SWIG_0(swigCPtr);
  }

  public void Reverse(int index, int count) {
    AssimpPINVOKE.aiAnimMeshVector_Reverse__SWIG_1(swigCPtr, index, count);
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetRange(int index, aiAnimMeshVector values) {
    AssimpPINVOKE.aiAnimMeshVector_SetRange(swigCPtr, index, aiAnimMeshVector.getCPtr(values));
    if (AssimpPINVOKE.SWIGPendingException.Pending) throw AssimpPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool Contains(aiAnimMesh value) {
    bool ret = AssimpPINVOKE.aiAnimMeshVector_Contains(swigCPtr, aiAnimMesh.getCPtr(value));
    return ret;
  }

  public int IndexOf(aiAnimMesh value) {
    int ret = AssimpPINVOKE.aiAnimMeshVector_IndexOf(swigCPtr, aiAnimMesh.getCPtr(value));
    return ret;
  }

  public int LastIndexOf(aiAnimMesh value) {
    int ret = AssimpPINVOKE.aiAnimMeshVector_LastIndexOf(swigCPtr, aiAnimMesh.getCPtr(value));
    return ret;
  }

  public bool Remove(aiAnimMesh value) {
    bool ret = AssimpPINVOKE.aiAnimMeshVector_Remove(swigCPtr, aiAnimMesh.getCPtr(value));
    return ret;
  }

}
