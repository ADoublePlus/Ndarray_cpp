This template library provides a Numpy Ndarray inspired multi-dimensional array class for C++.

# Ndarray Design

Ndarray objects effectively wraps a pointer to data, holds some meta data and allows to use that contiguous block
of data in a manner of a multi-dimensional array object. The wrapped data is shared by design, and managed using shared_ptr.
Subscripting and slicing returns views into the original data. Discontinuous slicing is available, following the same
strided approach as Numpy does.

If an existing pointer is wrapped with an Ndarray object, the memory will not be freed by default. There is however
the possibility to pass a destructor function, which will be called if the reference count reaches zero.
If no pointer is passed to the constructor, an array of the parameterized datatype will be used to store data and
is freed when no other Ndarray objects points into the data.
