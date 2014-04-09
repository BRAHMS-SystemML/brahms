


Layer: Public

This layer consists of the core and engine APIs.



Layer: Support

This layer is immediately on top of Public (core and engine APIs). Each component of the layer provides some types and/or operations (offered either through an object-oriented or functional interface).

These components are not stateful. Therefore, if they are to generate logging output, they must always be passed a brahms::output::Source to which they can send it. Errors in these components are handled by throwing a brahms::error::Error, which may have additional information attached.

This layer does, however, have some global states, described next.

Object Register: "register.h/.cpp" provides an object register that is global *across all engine instances*. Therefore, there is a danger of it becoming clogged if multiple engines are instantiated together in some particular environment. This is not going to happen in the current implementations, however. If this becomes a problem in future, it may be necessary to separate this support layer into two parts: a lower part, which has no state at all, and an upper part, which has state and is engine-aware, so that the upper part can maintain an object register at the engine level.

Error Register: "error.h/.cpp" provides an error register that is global *across all engine instances*. This is used to propagate errors across C interfaces, and is unlikely to become clogged, since such errors should be in C code only briefly before they are returned to the framework.


